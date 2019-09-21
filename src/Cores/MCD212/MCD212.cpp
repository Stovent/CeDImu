#include "MCD212.hpp"

#include <cstring>
#include <wx/msgdlg.h>

MCD212::MCD212(CeDImu* appp) : VDSC(appp) // TD = 0
{
    app = appp;
    registers = new uint32_t[0x80];
    internalRegisters = new uint16_t[32];
    memory = new uint8_t[0x500000];
    allocatedMemory = 0x500000;
    memorySwapCount = 0;
    biosLoaded = false;
#ifdef DEBUG
    out.open("MCD212.txt");
#endif // DEBUG
    cursorPlane = new wxImage(16, 16);
    internalRegisters[CSR1R] |= 0x80;
}

MCD212::~MCD212()
{
    delete[] memory;
}

void MCD212::Reset()
{
    registers[ImageCodingMethod] &= 0xF80000; // bits 0, 1, 2, 3, 8, 9, 10, 11, 18 (plane A and B off,external video disabled)
    registers[CursorControl] &= 0x7FFFFF; // bit 23 (cursor disabled)
    registers[BackdropColor] = 0; // bits 0, 1, 2, 3 (black backdrop)

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

uint32_t MCD212::GetLineDisplayTime() // as nano seconds
{
    return 700;
}

void MCD212::DisplayLine()
{
    if(!GetDE())
        return;

    DisplayLine1();
    DisplayLine2();

    if(lineNumber == 284)
    {
        wxImage* screen = new wxImage();
        app->mainFrame->gamePanel->RefreshScreen(screen);
    }
}

void MCD212::DisplayLine1()
{

}

void MCD212::DisplayLine2()
{
    uint32_t vsr = GetVSR2();
}

void MCD212::ExecuteICA1()
{
    uint32_t ica = 0x400;
    switch(ica >> 28)
    {
    case 0:
        break;

    case 1:
        break;

    case 2:
        break;

    case 3:
        break;

    case 4:
        break;

    case 5:
        break;

    case 6:
        break;

    case 7:
        break;

#ifdef DEBUG
    default:
        out << "Bad ICA1" << std::endl;
#endif // DEBUG
    }
}

void MCD212::ExecuteDCA1()
{
    uint32_t dcp = GetDCP1();
    switch(dcp >> 28)
    {
    case 0:
        break;

    case 1:
        break;

    case 2:
        break;

    case 3:
        break;

    case 4:
        break;

    case 5:
        break;

    case 6:
        break;

    case 7:
        break;

#ifdef DEBUG
    default:
        out << "Bad DCA1" << std::endl;
#endif // DEBUG
    }
}

void MCD212::ExecuteICA2()
{
    uint32_t ica  = 0x200400;
    switch(ica >> 28)
    {
    case 0:
        break;

    case 1:
        break;

    case 2:
        break;

    case 3:
        break;

    case 4:
        break;

    case 5:
        break;

    case 6:
        break;

    case 7:
        break;

#ifdef DEBUG
    default:
        out << "Bad ICA2" << std::endl;
#endif // DEBUG
    }
}

void MCD212::ExecuteDCA2()
{
    uint32_t dcp = GetDCP2();
    switch(dcp >> 28)
    {
    case 0:
        break;

    case 1:
        break;

    case 2:
        break;

    case 3:
        break;

    case 4:
        break;

    case 5:
        break;

    case 6:
        break;

    case 7:
        break;

#ifdef DEBUG
    default:
        out << "Bad DCA2" << std::endl;
#endif // DEBUG
    }
}
