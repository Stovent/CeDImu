#include "MCD212.hpp"

#include <cstring>
#include <wx/msgdlg.h>

#define   SET_DA_BIT() internalRegisters[CSR1R] |= 0x80;
#define UNSET_DA_BIT() internalRegisters[CSR1R] &= 0x20;

MCD212::MCD212(CeDImu* appp) : VDSC(appp) // TD = 0
{
    controlRegisters = new uint32_t[0x80];
    internalRegisters = new uint16_t[32];
    memory = new uint8_t[0x500000];
    allocatedMemory = 0x500000;
    memorySwapCount = 0;
    isCA = false;
#ifdef DEBUG
    out.open("MCD212.txt");
#endif // DEBUG
    cursorPlane = new wxImage(16, 16);
}

MCD212::~MCD212()
{
    delete[] memory;
}

void MCD212::Reset()
{
    controlRegisters[ImageCodingMethod] &= 0xF80000; // reset bits 0, 1, 2, 3, 8, 9, 10, 11, 18 (plane A and B off,external video disabled)
    controlRegisters[CursorControl] &= 0x7FFFFF; // reset bit 23 (cursor disabled)
    controlRegisters[BackdropColor] = 0; // reset bits 0, 1, 2, 3 (black backdrop)

    internalRegisters[CSR1W] = 0; // DI1, DD1, DD2, TD, DD, ST, BE
    internalRegisters[CSR2W] = 0; // DI2
    internalRegisters[DCR1] &= 0x003F; // DE, CF, FD, SM, CM1, IC1, DC1
    internalRegisters[DCR2] &= 0x003F; // CM2, IC2, DC2
    internalRegisters[DDR1] &= 0x003F; // MF1, MF2, FT1, FT2
    internalRegisters[DDR2] &= 0x003F; // MF1, MF2, FT1, FT2
}

bool MCD212::LoadBIOS(const char* filename)
{
    biosLoaded = false;
    FILE* f = fopen(filename, "rb");
    if(f == NULL)
        return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f); // should be 512KB
    fseek(f, 0, SEEK_SET);

    fread(&memory[0x400000], 1, size, f);

    fclose(f);
    return biosLoaded = true;
}

void MCD212::PutDataInMemory(const void* s, unsigned int size, unsigned int position)
{
    memcpy(&memory[position], s, size);
}

void MCD212::ResetMemory()
{
    memset(memory, 0, 0x500000);
}

void MCD212::MemorySwap()
{
    memorySwapCount = 0;
}

void MCD212::DisplayLine()
{
    SET_DA_BIT()

    if(GetDE())
    {
        DisplayLine1();
        DisplayLine2();
    }

    if(++lineNumber >= GetVerticalResolution())
    {
        lineNumber = 0;
        UNSET_DA_BIT()
        if(GetDE())
        {
//            wxImage* screen = new wxImage();
//            app->mainFrame->gamePanel->RefreshScreen(screen);
        }
    }
}

void MCD212::DisplayLine1()
{
    if(GetIC1() && GetDC1())
        ExecuteDCA1();

    if(GetIC1() && lineNumber >= GetVerticalResolution()-1)
        ExecuteICA1();
}

void MCD212::DisplayLine2()
{
    if(GetIC2() && GetDC2())
        ExecuteDCA2();

    if(GetIC2() && lineNumber >= GetVerticalResolution()-1)
        ExecuteICA2();
}

void MCD212::ExecuteICA1()
{
    isCA = true;
    uint32_t ica;
    uint32_t addr = 0x400;
    for(uint16_t i = 0; i < 2000; i++) // change 2000 with value in section 5.4.1
    {
        ica = GetLong(addr + 4*i);
        switch(ica >> 28)
        {
        case 0: // STOP
            goto end_ICA1;
            break;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP1(ica & 0x003FFFFC);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP1(ica & 0x003FFFFC);
            goto end_ICA1;
            break;

        case 4: // RELOAD ICA
            addr = ica & 0x003FFFFF;
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR1(ica & 0x003FFFFF);
            goto end_ICA1;
            break;

        case 6: // INTERRUPT
            SetIT1();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters1((ica >> 4) & 1, (ica >> 2) & 3, ica & 3);
            break;

        default:
            controlRegisters[(ica >> 24) - 0x80] = ica & 0x00FFFFFF;
        }
#ifdef DEBUG
        out << "ICA1 instruction: 0x" << std::hex << ica << std::endl;
#endif // DEBUG
    }
end_ICA1:
    isCA = false;
    return;
}

void MCD212::ExecuteDCA1()
{
    isCA = true;
    uint32_t dca;
    uint32_t addr = GetDCP1() + 64*lineNumber;
    for(uint8_t i = 0; i < 16; i++)
    {
        dca = GetLong(addr);
        switch(dca >> 28)
        {
        case 0: // STOP
            goto end_DCA1;
            break;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP1(dca & 0x003FFFFC);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP1(dca & 0x003FFFFC);
            goto end_DCA1;
            break;

        case 4: // RELOAD VSR
            SetVSR1(dca & 0x003FFFFF);
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR1(dca & 0x003FFFFF);
            goto end_DCA1;
            break;

        case 6: // INTERRUPT
            SetIT1();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters1((dca >> 4) & 1, (dca >> 2) & 3, dca & 3);
            break;

        default:
            controlRegisters[(dca >> 24) - 0x80] = dca & 0x00FFFFFF;
        }
#ifdef DEBUG
        out << "DCA1 instruction: 0x" << std::hex << dca << std::endl;
#endif // DEBUG
        addr += 4;
    }
end_DCA1:
    isCA = false;
    return;
}

void MCD212::ExecuteICA2()
{
    isCA = true;
    uint32_t ica;
    uint32_t addr = 0x200400;
    for(uint16_t i = 0; i < 2000; i++) // change 2000 with value in section 5.4.1
    {
        ica = GetLong(addr + 4*i);
        switch(ica >> 28)
        {
        case 0: // STOP
            goto end_ICA2;
            break;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP2(ica & 0x003FFFFC);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP2(ica & 0x003FFFFC);
            goto end_ICA2;
            break;

        case 4: // RELOAD ICA
            addr = ica & 0x003FFFFF;
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR2(ica & 0x003FFFFF);
            goto end_ICA2;
            break;

        case 6: // INTERRUPT
            SetIT2();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters2((ica >> 4) & 1, (ica >> 2) & 3, ica & 3);
            break;

        default:
            controlRegisters[(ica >> 24) - 0x80] = ica & 0x00FFFFFF;
        }
#ifdef DEBUG
        out << "ICA2 instruction: 0x" << std::hex << ica << std::endl;
#endif // DEBUG
    }
end_ICA2:
    isCA = false;
    return;
}

void MCD212::ExecuteDCA2()
{
    isCA = true;
    uint32_t dca;
    uint32_t addr = GetDCP2() + 64*lineNumber;
    for(uint8_t i = 0; i < 16; i++)
    {
        dca = GetLong(addr);
        switch(dca >> 28)
        {
        case 0: // STOP
            goto end_DCA2;
            break;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP2(dca & 0x003FFFFC);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP2(dca & 0x003FFFFC);
            goto end_DCA2;
            break;

        case 4: // RELOAD VSR
            SetVSR2(dca & 0x003FFFFF);
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR2(dca & 0x003FFFFF);
            goto end_DCA2;
            break;

        case 6: // INTERRUPT
            SetIT2();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters2((dca >> 4) & 1, (dca >> 2) & 3, dca & 3);
            break;

        default:
            controlRegisters[(dca >> 24) - 0x80] = dca & 0x00FFFFFF;
        }
#ifdef DEBUG
        out << "DCA2 instruction: 0x" << std::hex << dca << std::endl;
#endif // DEBUG
        addr += 4;
    }
end_DCA2:
    isCA = false;
    return;
}
