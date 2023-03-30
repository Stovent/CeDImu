#include "MCD212.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"
#include "../../common/Video.hpp"

#include <algorithm>
#include <cstring>

MCD212::MCD212(CDI& idc, std::span<const uint8_t> systemBios, const bool PAL)
    : BIOS(systemBios, 0x400000)
    , totalFrameCount(0)
    , cdi(idc)
    , isPAL(PAL)
    , memorySwapCount(0)
    , timeNs(0.0)
    , memory(0x280000, 0)
    , screen(Video::Plane::RGB_SIZE)
    , planeA()
    , planeB()
    , cursorPlane(Video::Plane::CURSOR_ARGB_SIZE, 16, 16)
    , backgroundPlane(Video::Plane::MAX_HEIGHT * 4, 1, Video::Plane::MAX_HEIGHT)
    , controlRegisters{0}
    , CLUT{0}
    , cursorPatterns{0}
    , regionFlags{0}
    , currentRegionControl(RegionControl - 1)
    , internalRegisters{0}
    , registerCSR1R(0)
    , registerCSR2R(0)
    , lineNumber(0)
    , verticalLines(0)
{
}

MCD212::~MCD212()
{
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
    registerCSR1R = 0;
    registerCSR2R = 0;

    std::fill(screen.begin(), screen.end(), 0);

    verticalLines = 0;
    lineNumber = 0;
    timeNs = 0.0;
    MemorySwap();
}

void MCD212::IncrementTime(const double ns)
{
    timeNs += ns;
    const size_t lineDisplayTime = GetLineDisplayTime();
    if(timeNs >= lineDisplayTime)
    {
        ExecuteVideoLine();
        timeNs -= lineDisplayTime;
    }
}

void MCD212::MemorySwap()
{
    memorySwapCount = 0;
}

void MCD212::ExecuteICA1()
{
    const size_t cycles = GetHorizontalCycles() * GetVerticalRetraceLines();
    uint32_t addr = GetSM() ? (GetPA() ? 0x400 : 0x404) : 0x400;
    for(size_t i = 0; i < cycles; i++)
    {
        const uint32_t ica = GetLong(addr);

        if(cdi.m_callbacks.HasOnLogICADCA())
            cdi.m_callbacks.OnLogICADCA(Video::ControlArea::ICA1, { totalFrameCount + 1, 0, ica });
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
                cdi.m_cpu.INT1();
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
        SetDCP1(addr + 4);

        if(cdi.m_callbacks.HasOnLogICADCA())
            cdi.m_callbacks.OnLogICADCA(Video::ControlArea::DCA1, { totalFrameCount + 1, lineNumber, dca });

        switch(dca >> 28)
        {
        case 0: // STOP
            return;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP1(dca & 0x003FFFFC);
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
                cdi.m_cpu.INT1();
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
    }
}

void MCD212::ExecuteICA2()
{
    const size_t cycles = GetHorizontalCycles() * GetVerticalRetraceLines();
    uint32_t addr = GetSM() ? (GetPA() ? 0x200400 : 0x200404) : 0x200400;
    for(size_t i = 0; i < cycles; i++)
    {
        const uint32_t ica = GetLong(addr);

        if(cdi.m_callbacks.HasOnLogICADCA())
            cdi.m_callbacks.OnLogICADCA(Video::ControlArea::ICA2, { totalFrameCount + 1, 0, ica });
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
                cdi.m_cpu.INT1();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters2((ica >> 4) & 1, (ica >> 2) & 3, ica & 3);
            break;

        default:
            if(ica < 0xC0000000) // CLUT RAM
            {
                const uint8_t bank = controlRegisters[CLUTBank] << 6;
                LOG(if(bank < 2) { fprintf(stderr, "WARNING: writing CLUT bank %d from channel #2 is forbidden!\n", bank); })
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
        SetDCP2(addr + 4);

        if(cdi.m_callbacks.HasOnLogICADCA())
            cdi.m_callbacks.OnLogICADCA(Video::ControlArea::DCA2, { totalFrameCount + 1, lineNumber, dca });

        switch(dca >> 28)
        {
        case 0: // STOP
            return;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP2(dca & 0x003FFFFC);
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
                cdi.m_cpu.INT1();
            break;

        case 7: // RELOAD DISPLAY PARAMETERS
            ReloadDisplayParameters2((dca >> 4) & 1, (dca >> 2) & 3, dca & 3);
            break;

        default:
            if(dca < 0xC0000000) // CLUT RAM
            {
                const uint8_t bank = controlRegisters[CLUTBank] << 6;
                LOG(if(bank < 2) { fprintf(stderr, "WARNING: writing CLUT bank %d from channel #2 is forbidden!\n", bank); })
                const uint8_t index = (uint8_t)(dca >> 24) - 0x80;
                CLUT[bank + index] = dca & 0x00FFFFFF;
            }
            else
                controlRegisters[(uint8_t)(dca >> 24) - 0x80] = dca & 0x00FFFFFF;
        }
    }
}

RAMBank MCD212::GetRAMBank1() const
{
    return {{memory.data(), 0x80000}, 0};
}

RAMBank MCD212::GetRAMBank2() const
{
    return {{&memory[0x200000], 0x80000}, 0x200000};
}

const Video::Plane& MCD212::GetScreen() const
{
    return screen;
}

const Video::Plane& MCD212::GetPlaneA() const
{
    return planeA;
}

const Video::Plane& MCD212::GetPlaneB() const
{
    return planeB;
}

const Video::Plane& MCD212::GetBackground() const
{
    return backgroundPlane;
}

const Video::Plane& MCD212::GetCursor() const
{
    return cursorPlane;
}
