#include "MCD212.hpp"

#include <iomanip>

#include <wx/msgdlg.h>

MCD212::MCD212(CeDImu* appp) : VDSC(appp) // TD = 0
{
    controlRegisters = new uint32_t[0x80];
    internalRegisters = new uint16_t[32];
    allocatedMemory = 0x500000;
    memory = new uint8_t[allocatedMemory];
    memorySwapCount = 0;

    OPEN_LOG(out_dram, "MCD212_DRAM.txt")
    OPEN_LOG(out_display, "MCD212.txt")

    cursorPlane.Create(16, 16);
    if(!cursorPlane.HasAlpha())
        cursorPlane.InitAlpha();

    memset(controlRegisters, 0, 0x80*sizeof(uint32_t));
    memset(internalRegisters, 0, 32*sizeof(uint16_t));
    memset(memory, 0, allocatedMemory);
    memset(CLUT, 0, 256 * sizeof(uint32_t));
    Reset();
}

MCD212::~MCD212()
{
    delete[] controlRegisters;
    delete[] internalRegisters;
}

void MCD212::Reset()
{
    controlRegisters[ImageCodingMethod] &= 0x480000; // reset bits 0, 1, 2, 3, 8, 9, 10, 11, 18 (plane A and B off, external video disabled)
    controlRegisters[CursorControl] &= 0x7F800F; // reset bit 23 (cursor disabled)
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

    if(size > 0xFFC00)
    {
        LOG(out_dram << "WARNING: BIOS is bigger than ROM size (0xFFC00)" << std::endl)
        wxMessageBox("BIOS is bigger than ROM size (0xFFC00)");
        size = 0xFFC00;
    }

    fread(&memory[0x400000], 1, size, f);

    fclose(f);
    std::string str(filename);
#ifdef _WIN32
    biosFilename = str.substr(str.rfind('\\')+1);
#else
    biosFilename = str.substr(str.rfind('/')+1);
#endif // _WIN32
    return biosLoaded = true;
}

void MCD212::PutDataInMemory(const void* s, unsigned int size, unsigned int position)
{
    memcpy(&memory[position], s, size);
}

void MCD212::MemorySwap()
{
    memorySwapCount = 0;
}

void MCD212::ExecuteICA1()
{
    uint32_t addr = 0x400;
    for(uint16_t i = 0; i < 2000; i++) // change 2000 with value in section 5.4.1
    {
        const uint32_t ica = GetLong(addr);

        LOG(out_display << std::setw(6) << std::setfill(' ') << std::hex << addr; \
            out_display << "\tFrame: " << std::setw(6) << std::setfill(' ') << std::dec << totalFrameCount; \
            out_display << "\tLine: " << std::setw(3) << std::setfill(' ') << std::dec << lineNumber; \
            out_display << "\tICA1 instruction: 0x" << std::setw(8) << std::setfill('0') << std::hex << ica << std::endl)
        ICA1.push_back("Frame " + std::to_string(totalFrameCount) + "\tline " + std::to_string (lineNumber) + "\t : 0x" + toHex(ica));

        switch(ica >> 28)
        {
        case 0: // STOP
            return;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP1(ica & 0x003FFFFC);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP1(ica & 0x003FFFFC);
            return;

        case 4: // RELOAD ICA
            addr = (ica & 0x003FFFFF) - 4;
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR1(ica & 0x003FFFFF);
            return;

        case 6: // INTERRUPT
            SetIT1();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters1((ica >> 4) & 1, (ica >> 2) & 3, ica & 3);
            break;

        default:
            if(ica < 0xC0000000) // CLUT RAM
            {
                const uint8_t bank = controlRegisters[CLUTBank] << 6;
                const uint8_t index = (uint8_t)(ica >> 24) - 0x80;
                CLUT[bank + index] = ica & 0x00FFFFFF;
            }
            else
                controlRegisters[(uint8_t)(ica >> 24) - 0x80] = ica & 0x00FFFFFF;
        }
        addr += 4;
    }
}

void MCD212::ExecuteDCA1()
{
    for(uint8_t i = 0; i < 16; i++) // 64 bytes/line
    {
        const uint32_t addr = GetDCP1();
        const uint32_t dca = GetLong(addr);

        LOG(out_display << std::setw(6) << std::setfill(' ') << std::hex << addr; \
            out_display << "\tFrame: " << std::setw(6) << std::setfill(' ') << std::dec << totalFrameCount; \
            out_display << "\tLine: " << std::setw(3) << std::setfill(' ') << std::dec << lineNumber; \
            out_display << "\tDCA1 instruction: 0x" << std::setw(8) << std::setfill('0') << std::hex << dca << std::endl)
        DCA1.push_back("Frame " + std::to_string(totalFrameCount) + "\tline " + std::to_string (lineNumber) + "\t : 0x" + toHex(dca));

        switch(dca >> 28)
        {
        case 0: // STOP
            return;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP1((dca & 0x003FFFFC) - 4);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP1(dca & 0x003FFFFC);
            return;

        case 4: // RELOAD VSR
            SetVSR1(dca & 0x003FFFFF);
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR1(dca & 0x003FFFFF);
            return;

        case 6: // INTERRUPT
            SetIT1();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters1((dca >> 4) & 1, (dca >> 2) & 3, dca & 3);
            break;

        default:
            if(dca < 0xC0000000) // CLUT RAM
            {
                const uint8_t bank = controlRegisters[CLUTBank] << 6;
                const uint8_t index = (uint8_t)(dca >> 24) - 0x80;
                CLUT[bank + index] = dca & 0x00FFFFFF;
            }
            else
                controlRegisters[(uint8_t)(dca >> 24) - 0x80] = dca & 0x00FFFFFF;
        }
        SetDCP1(addr + 4);
    }
}

void MCD212::ExecuteICA2()
{
    uint32_t addr = 0x200400;
    for(uint16_t i = 0; i < 2000; i++) // change 2000 with value in section 5.4.1
    {
        const uint32_t ica = GetLong(addr);

        LOG(out_display << std::setw(6) << std::setfill(' ') << std::hex << addr; \
            out_display << "\tFrame: " << std::setw(6) << std::setfill(' ') << std::dec << totalFrameCount; \
            out_display << "\tLine: " << std::setw(3) << std::setfill(' ') << std::dec << lineNumber; \
            out_display << "\tICA2 instruction: 0x" << std::setw(8) << std::setfill('0') << std::hex << ica << std::endl)
        ICA2.push_back("Frame " + std::to_string(totalFrameCount) + "\tline " + std::to_string (lineNumber) + "\t : 0x" + toHex(ica));

        switch(ica >> 28)
        {
        case 0: // STOP
            return;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP2(ica & 0x003FFFFC);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP2(ica & 0x003FFFFC);
            return;

        case 4: // RELOAD ICA
            addr = (ica & 0x003FFFFF) - 4;
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR2(ica & 0x003FFFFF);
            return;

        case 6: // INTERRUPT
            SetIT2();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters2((ica >> 4) & 1, (ica >> 2) & 3, ica & 3);
            break;

        default:
            if(ica < 0xC0000000) // CLUT RAM
            {
                const uint8_t bank = controlRegisters[CLUTBank] << 6;
                LOG(if(bank > 1) { out_display << "WARNING: writing CLUT bank " << (int)bank << " from channel #2 is forbidden!" << std::endl;})
                const uint8_t index = (uint8_t)(ica >> 24) - 0x80;
                CLUT[bank + index] = ica & 0x00FFFFFF;
            }
            else
                controlRegisters[(uint8_t)(ica >> 24) - 0x80] = ica & 0x00FFFFFF;
        }
        addr += 4;
    }
}

void MCD212::ExecuteDCA2()
{
    for(uint8_t i = 0; i < 16; i++)
    {
        const uint32_t addr = GetDCP2();
        const uint32_t dca = GetLong(addr);

        LOG(out_display << std::setw(6) << std::setfill(' ') << std::hex << addr; \
            out_display << "\tFrame: " << std::setw(6) << std::setfill(' ') << std::dec << totalFrameCount; \
            out_display << "\tLine: " << std::setw(3) << std::setfill(' ') << std::dec << lineNumber; \
            out_display << "\tDCA2 instruction: 0x" << std::setw(8) << std::setfill('0') << std::hex << dca << std::endl)
        DCA2.push_back("Frame " + std::to_string(totalFrameCount) + "\tline " + std::to_string (lineNumber) + "\t : 0x" + toHex(dca));

        switch(dca >> 28)
        {
        case 0: // STOP
            return;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP2((dca & 0x003FFFFC) - 4);
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP2(dca & 0x003FFFFC);
            return;

        case 4: // RELOAD VSR
            SetVSR2(dca & 0x003FFFFF);
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR2(dca & 0x003FFFFF);
            return;

        case 6: // INTERRUPT
            SetIT2();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters2((dca >> 4) & 1, (dca >> 2) & 3, dca & 3);
            break;

        default:
            if(dca < 0xC0000000) // CLUT RAM
            {
                const uint8_t bank = controlRegisters[CLUTBank] << 6;
                LOG(if(bank > 1) { out_display << "WARNING: writing CLUT bank " << (int)bank << " from channel #2 is forbidden!" << std::endl;})
                const uint8_t index = (uint8_t)(dca >> 24) - 0x80;
                CLUT[bank + index] = dca & 0x00FFFFFF;
            }
            else
                controlRegisters[(uint8_t)(dca >> 24) - 0x80] = dca & 0x00FFFFFF;
        }
        SetDCP2(addr + 4);
    }
}

void MCD212::OnFrameCompleted()
{
    if(stopOnNextCompletedFrame)
    {
        stopOnNextCompletedFrame = false;
        app->cpu->run = false;
        app->mainFrame->pauseItem->Check(true);
        app->mainFrame->SetStatusText("pause");
    }
}

wxImage MCD212::GetPlaneA()
{
    return planeA;
}

wxImage MCD212::GetPlaneB()
{
    return planeB;
}

wxImage MCD212::GetBackground()
{
    return backgroundPlane;
}

wxImage MCD212::GetCursor()
{
    return cursorPlane;
}
