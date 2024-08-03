#include "MCD212.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <cstring>

#define DCP_PARAMETER(inst) (bits<0, 23>(inst))
#define DCP_POINTER(inst) (inst & 0x003FFFFCu)
#define ICA_VSR_POINTER(inst) (bits<0, 21>(inst))

MCD212::MCD212(CDI& idc, OS9::BIOS bios, const bool pal)
    : BIOS(std::move(bios))
    , totalFrameCount(0)
    , cdi(idc)
    , isPAL(pal)
    , memorySwapCount(0)
    , timeNs(0.0)
    , renderer()
    , memory(0x280000, 0)
    , internalRegisters{0}
    , registerCSR1R(0)
    , registerCSR2R(0)
    , verticalLines(0)
{
}

void MCD212::Reset()
{
    renderer.m_externalVideo = false; // reset bits 0, 1, 2, 3, 8, 9, 10, 11, 18 (plane A and B off, external video disabled)
    renderer.m_codingMethod[PlaneA] = Video::ImageCodingMethod::OFF;
    renderer.m_codingMethod[PlaneB] = Video::ImageCodingMethod::OFF;
    renderer.SetCursorEnabled(false); // reset bit 23 (cursor disabled)
    renderer.m_backdropColor = 0; // reset bits 0, 1, 2, 3 (black backdrop)

    // TODO: reset those too.
    internalRegisters[CSR1W] = 0; // DI1, DD1, DD2, TD, DD, ST, BE
    internalRegisters[CSR2W] = 0; // DI2
    internalRegisters[DCR1] &= 0x003F; // DE, CF, FD, SM, CM1, IC1, DC1
    internalRegisters[DCR2] &= 0x003F; // CM2, IC2, DC2
    internalRegisters[DDR1] &= 0x003F; // MF1, MF2, FT1, FT2
    internalRegisters[DDR2] &= 0x003F; // MF1, MF2, FT1, FT2
    registerCSR1R = 0;
    registerCSR2R = 0;

    verticalLines = 0;
    timeNs = 0.0;
    MemorySwap();
}

void MCD212::IncrementTime(const double ns)
{
    timeNs += ns;
    const size_t lineDisplayTime = GetLineDisplayTime();
    if(timeNs >= lineDisplayTime)
    {
        DrawVideoLine();
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
        const uint32_t ica = GetControlInstruction(addr);

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
            SetDCP1(DCP_POINTER(ica));
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP1(DCP_POINTER(ica));
            return;

        case 4: // RELOAD ICA
            addr = ICA_VSR_POINTER(ica);
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR1(ICA_VSR_POINTER(ica));
            return;

        case 6: // INTERRUPT
            SetIT1();
            if(!GetDI1())
                cdi.m_cpu.INT1();
            break;
        }

        const uint8_t code = ica >> 24;
        switch(code)
        {
        case 0xCD:
            renderer.SetCursorPosition(bits<1, 9>(ica), bits<12, 21>(ica)); // Double resolution.
            break;

        case 0xCE:
            renderer.SetCursorColor(bits<0, 3>(ica));
            renderer.SetCursorEnabled(bit<23>(ica));
            break;

        case 0xCF:
            renderer.SetCursorPattern(bits<16, 19>(ica), ica);
            break;

        case 0x70:
            // This command is implemented in the SCC66470, but the driver still sends it to the MCD212, so just ignore it.
            break;

        default:
            renderer.ExecuteDCPInstruction<Video::Renderer::A>(ica);
            break;
        }
    }
}

void MCD212::ExecuteDCA1()
{
    for(uint8_t i = 0; i < (GetCF() ? 16 : 8); i++) // Table 5.10
    {
        const uint32_t addr = GetDCP1();
        const uint32_t dca = GetControlInstruction(addr);
        SetDCP1(addr + 4);

        if(cdi.m_callbacks.HasOnLogICADCA())
            cdi.m_callbacks.OnLogICADCA(Video::ControlArea::DCA1, { totalFrameCount + 1, renderer.m_lineNumber, dca });

        switch(dca >> 28)
        {
        case 0: // STOP
            return;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP1(DCP_POINTER(dca));
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP1(DCP_POINTER(dca));
            return;

        case 4: // RELOAD VSR
            SetVSR1(ICA_VSR_POINTER(dca));
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR1(ICA_VSR_POINTER(dca));
            return;

        case 6: // INTERRUPT
            SetIT1();
            if(!GetDI1())
                cdi.m_cpu.INT1();
            break;
        }

        const uint8_t code = dca >> 24;
        switch(code)
        {
        case 0xCD:
            renderer.SetCursorPosition(bits<1, 9>(dca), bits<12, 21>(dca)); // Double resolution.
            break;

        case 0xCE:
            renderer.SetCursorColor(bits<0, 3>(dca));
            renderer.SetCursorEnabled(bit<23>(dca));
            break;

        case 0xCF:
            renderer.SetCursorPattern(bits<16, 19>(dca), dca);
            break;

        default:
            renderer.ExecuteDCPInstruction<Video::Renderer::A>(dca);
            break;
        }
    }
}

void MCD212::ExecuteICA2()
{
    const size_t cycles = GetHorizontalCycles() * GetVerticalRetraceLines();
    uint32_t addr = GetSM() ? (GetPA() ? 0x200400 : 0x200404) : 0x200400;
    for(size_t i = 0; i < cycles; i++)
    {
        const uint32_t ica = GetControlInstruction(addr);

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
            SetDCP2(DCP_POINTER(ica));
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP2(DCP_POINTER(ica));
            return;

        case 4: // RELOAD ICA
            addr = ICA_VSR_POINTER(ica);
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR2(ICA_VSR_POINTER(ica));
            return;

        case 6: // INTERRUPT
            SetIT2();
            if(!GetDI2())
                cdi.m_cpu.INT1();
            break;
        }

        // This command is implemented in the SCC66470, but the driver still sends it to the MCD212, so just ignore it.
        if(ica != 0x70000000)
            renderer.ExecuteDCPInstruction<Video::Renderer::B>(ica);
    }
}

void MCD212::ExecuteDCA2()
{
    for(uint8_t i = 0; i < (GetCF() ? 16 : 8); i++) // Table 5.10
    {
        const uint32_t addr = GetDCP2();
        const uint32_t dca = GetControlInstruction(addr);
        SetDCP2(addr + 4);

        if(cdi.m_callbacks.HasOnLogICADCA())
            cdi.m_callbacks.OnLogICADCA(Video::ControlArea::DCA2, { totalFrameCount + 1, renderer.m_lineNumber, dca });

        switch(dca >> 28)
        {
        case 0: // STOP
            return;

        case 1: // NOP
            break;

        case 2: // RELOAD DCP
            SetDCP2(DCP_POINTER(dca));
            break;

        case 3: // RELOAD DCP AND STOP
            SetDCP2(DCP_POINTER(dca));
            return;

        case 4: // RELOAD VSR
            SetVSR2(ICA_VSR_POINTER(dca));
            break;

        case 5: // RELOAD VSR AND STOP
            SetVSR2(ICA_VSR_POINTER(dca));
            return;

        case 6: // INTERRUPT
            SetIT2();
            if(!GetDI2())
                cdi.m_cpu.INT1();
            break;
        }

        renderer.ExecuteDCPInstruction<Video::Renderer::B>(dca);
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
    return renderer.m_screen;
}

const Video::Plane& MCD212::GetPlaneA() const
{
    return renderer.m_plane[Video::Renderer::A];
}

const Video::Plane& MCD212::GetPlaneB() const
{
    return renderer.m_plane[Video::Renderer::B];
}

const Video::Plane& MCD212::GetBackground() const
{
    return renderer.m_backdropPlane;
}

const Video::Plane& MCD212::GetCursor() const
{
    return renderer.m_cursorPlane;
}
