#include "MCD212.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"
#include "../../common/Video.hpp"

#include <algorithm>
#include <cstring>

MCD212::MCD212(CDI& idc, const void* bios, const uint32_t size, const bool PAL) :
    VDSC(idc, bios, size, 0x400000),
    isPAL(PAL),
    memory(0x280000, 0),
    screen(PLANE_RGB_SIZE, 0),
    planeA(PLANE_ARGB_SIZE, 0),
    planeB(PLANE_ARGB_SIZE, 0),
    cursorPlane(CURSOR_ARGB_SIZE, 0),
    backgroundPlane(PLANE_ARGB_SIZE, 0)
{
    memorySwapCount = 0;

    std::fill(internalRegisters.begin(), internalRegisters.end(), 0);
    std::fill(CLUT.begin(), CLUT.end(), 0);
    std::fill(cursorPatterns.begin(), cursorPatterns.end(), 0);

    OPEN_LOG(out_dram, "MCD212_DRAM.txt")
    OPEN_LOG(out_display, "MCD212.txt")
}

MCD212::~MCD212()
{
    CLOSE_LOG(out_dram)
    CLOSE_LOG(out_display)
}

void MCD212::Reset()
{
    controlRegisters[ImageCodingMethod] &= 0x480000; // reset bits 0, 1, 2, 3, 8, 9, 10, 11, 18 (plane A and B off, external video disabled)
    controlRegisters[CursorControl] &= 0x7F800F; // reset bit 23 (cursor disabled)
    controlRegisters[BackdropColor] = 0; // reset bits 0, 1, 2, 3 (black backdrop)

    internalRegisters[MCSR1W] = 0; // DI1, DD1, DD2, TD, DD, ST, BE
    internalRegisters[MCSR2W] = 0; // DI2
    internalRegisters[MDCR1] &= 0x003F; // DE, CF, FD, SM, CM1, IC1, DC1
    internalRegisters[MDCR2] &= 0x003F; // CM2, IC2, DC2
    internalRegisters[MDDR1] &= 0x003F; // MF1, MF2, FT1, FT2
    internalRegisters[MDDR2] &= 0x003F; // MF1, MF2, FT1, FT2
    registerCSR1R = 0;
    registerCSR2R = 0;

    std::fill(screen.begin(), screen.end(), 0);

    verticalLines = 0;
    lineNumber = 0;
    MemorySwap();
}

void MCD212::PutDataInMemory(const void* s, unsigned int size, unsigned int position)
{
    memcpy(&memory[position], s, size);
}

void MCD212::WriteToBIOSArea(const void* s, unsigned int size, unsigned int position)
{
    PutDataInMemory(s, size, position + 0x400000);
}

void MCD212::MemorySwap()
{
    memorySwapCount = 0;
}

void MCD212::ExecuteICA1()
{
    uint32_t addr = GetSM() ? (GetPA() ? 0x400 : 0x404) : 0x400;
    for(uint16_t i = 0; i < 2000; i++) // change 2000 with value in section 5.4.1
    {
        const uint32_t ica = GetLong(addr);

        LOG(fprintf(out_display, "%6X\tFrame: %6d\tLine: %3d\tICA1 instruction: 0x%08X\n", addr, totalFrameCount, lineNumber, ica);)
        ICA1.push_back("Frame " + std::to_string(totalFrameCount) + "  line " + std::to_string (lineNumber) + ": 0x" + toHex(ica));
        addr += 4;

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
            addr = ica & 0x003FFFFF;
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR1(ica & 0x003FFFFF);
            return;

        case 6: // INTERRUPT
            SetIT1();
            if(!GetDI1())
                cdi.board->cpu.INT1();
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
            else if((ica & 0xFF000000) == 0xCF000000) // Cursor Pattern
            {
                const uint8_t reg = ica >> 16 & 0xF;
                cursorPatterns[reg] = ica;
                controlRegisters[CursorPattern] = ica & 0x00FFFFFF;
            }
            else
            {
                controlRegisters[(uint8_t)(ica >> 24) - 0x80] = ica & 0x00FFFFFF;
            }
        }
    }
}

void MCD212::ExecuteDCA1()
{
    for(uint8_t i = 0; i < (GetCF() ? 16 : 8); i++) // Table 5.10
    {
        const uint32_t addr = GetDCP1();
        const uint32_t dca = GetLong(addr);

        LOG(fprintf(out_display, "%6X\tFrame: %6d\tLine: %3d\tDCA1 instruction: 0x%08X\n", addr, totalFrameCount, lineNumber, dca);)
        DCA1.push_back("Frame " + std::to_string(totalFrameCount) + "  line " + std::to_string (lineNumber) + ": 0x" + toHex(dca));

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
            if(!GetDI1())
                cdi.board->cpu.INT1();
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
            else if((dca & 0xFF000000) == 0xCF000000) // Cursor Pattern
            {
                const uint8_t reg = dca >> 16 & 0xF;
                cursorPatterns[reg] = dca;
                controlRegisters[CursorPattern] = dca & 0x00FFFFFF;
            }
            else
            {
                controlRegisters[(uint8_t)(dca >> 24) - 0x80] = dca & 0x00FFFFFF;
            }
        }
        SetDCP1(addr + 4);
    }
}

void MCD212::ExecuteICA2()
{
    uint32_t addr = GetSM() ? (GetPA() ? 0x200400 : 0x200404) : 0x200400;
    for(uint16_t i = 0; i < 2000; i++) // change 2000 with value in section 5.4.1
    {
        const uint32_t ica = GetLong(addr);

        LOG(fprintf(out_display, "%6X\tFrame: %6d\tLine: %3d\tICA2 instruction: 0x%08X\n", addr, totalFrameCount, lineNumber, ica);)
        ICA2.push_back("Frame " + std::to_string(totalFrameCount) + "  line " + std::to_string (lineNumber) + ": 0x" + toHex(ica));
        addr += 4;

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
            addr = ica & 0x003FFFFF;
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR2(ica & 0x003FFFFF);
            return;

        case 6: // INTERRUPT
            SetIT2();
            if(!GetDI2())
                cdi.board->cpu.INT2();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters2((ica >> 4) & 1, (ica >> 2) & 3, ica & 3);
            break;

        default:
            if(ica < 0xC0000000) // CLUT RAM
            {
                const uint8_t bank = controlRegisters[CLUTBank] << 6;
                LOG(if(bank > 1) { fprintf(out_display, "WARNING: writing CLUT bank %d from channel #2 is forbidden!\n", bank); })
                const uint8_t index = (uint8_t)(ica >> 24) - 0x80;
                CLUT[bank + index] = ica & 0x00FFFFFF;
            }
            else
                controlRegisters[(uint8_t)(ica >> 24) - 0x80] = ica & 0x00FFFFFF;
        }
    }
}

void MCD212::ExecuteDCA2()
{
    for(uint8_t i = 0; i < (GetCF() ? 16 : 8); i++) // Table 5.10
    {
        const uint32_t addr = GetDCP2();
        const uint32_t dca = GetLong(addr);

        LOG(fprintf(out_display, "%6X\tFrame: %6d\tLine: %3d\tDCA2 instruction: 0x%08X\n", addr, totalFrameCount, lineNumber, dca);)
        DCA2.push_back("Frame " + std::to_string(totalFrameCount) + "  line " + std::to_string (lineNumber) + ": 0x" + toHex(dca));

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
            if(!GetDI2())
                cdi.board->cpu.INT2();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters2((dca >> 4) & 1, (dca >> 2) & 3, dca & 3);
            break;

        default:
            if(dca < 0xC0000000) // CLUT RAM
            {
                const uint8_t bank = controlRegisters[CLUTBank] << 6;
                LOG(if(bank > 1) { fprintf(out_display, "WARNING: writing CLUT bank %d from channel #2 is forbidden!\n", bank); })
                const uint8_t index = (uint8_t)(dca >> 24) - 0x80;
                CLUT[bank + index] = dca & 0x00FFFFFF;
            }
            else
                controlRegisters[(uint8_t)(dca >> 24) - 0x80] = dca & 0x00FFFFFF;
        }
        SetDCP2(addr + 4);
    }
}

RAMBank MCD212::GetRAMBank1() const
{
    return {memory.data(), 0, 0x80000};
}

RAMBank MCD212::GetRAMBank2() const
{
    return {&memory[0x200000], 0x200000, 0x80000};
}

Plane MCD212::GetScreen() const
{
    return {screen.data(), GetHorizontalResolution1(), GetVerticalResolution()};
}

Plane MCD212::GetPlaneA() const
{
    return {planeA.data(), GetHorizontalResolution1(), GetVerticalResolution()};
}

Plane MCD212::GetPlaneB() const
{
    return {planeB.data(), GetHorizontalResolution2(), GetVerticalResolution()};
}

Plane MCD212::GetBackground() const
{
    return {backgroundPlane.data(), GetHorizontalResolution1(), GetVerticalResolution()};
}

Plane MCD212::GetCursor() const
{
    return {cursorPlane.data(), 16, 16};
}
