#include "MCD212.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <cstring>

#define DCP_POINTER(inst) (inst & 0x003FFFFCu)
#define ICA_VSR_POINTER(inst) (bits<0, 21>(inst))

MCD212::MCD212(CDI& cdi, OS9::BIOS bios, const bool pal)
    : m_bios(std::move(bios))
    , m_cdi(cdi)
    , m_isPAL(pal)
    , m_memory(0x280000, 0)
{
}

void MCD212::Reset() noexcept
{
    m_renderer.m_externalVideo = false; // reset bits 0, 1, 2, 3, 8, 9, 10, 11, 18 (plane A and B off, external video disabled)
    m_renderer.m_codingMethod[PlaneA] = Video::ImageCodingMethod::OFF;
    m_renderer.m_codingMethod[PlaneB] = Video::ImageCodingMethod::OFF;
    m_renderer.SetCursorEnabled(false); // reset bit 23 (cursor disabled)
    m_renderer.m_backdropColor = 0; // reset bits 0, 1, 2, 3 (black backdrop)

    // TODO: reset those too.
    m_internalRegisters[CSR1W] = 0; // DI1, DD1, DD2, TD, DD, ST, BE
    m_internalRegisters[CSR2W] = 0; // DI2
    m_internalRegisters[DCR1] &= 0x003F; // DE, CF, FD, SM, CM1, IC1, DC1
    m_internalRegisters[DCR2] &= 0x003F; // CM2, IC2, DC2
    m_internalRegisters[DDR1] &= 0x003F; // MF1, MF2, FT1, FT2
    m_internalRegisters[DDR2] &= 0x003F; // MF1, MF2, FT1, FT2
    m_registerCSR1R = 0;
    m_registerCSR2R = 0;

    m_verticalLines = 0;
    m_timeNs = 0.0;
    m_totalFrameCount = 0;
    ResetMemorySwap();
}

void MCD212::IncrementTime(const double ns)
{
    m_timeNs += ns;
    const size_t lineDisplayTime = GetLineDisplayTime();
    if(m_timeNs >= lineDisplayTime)
    {
        DrawVideoLine();
        m_timeNs -= lineDisplayTime;
    }
}

void MCD212::ResetMemorySwap() noexcept
{
    m_memorySwapCount = 0;
}

void MCD212::ExecuteICA1()
{
    const size_t cycles = GetHorizontalCycles() * GetVerticalRetraceLines();
    uint32_t addr = GetSM() && !GetPA() ? 0x404 : 0x400;

    for(size_t i = 0; i < cycles; i++)
    {
        const uint32_t ica = GetControlInstruction(addr);

        if(m_cdi.m_callbacks.HasOnLogICADCA())
            m_cdi.m_callbacks.OnLogICADCA(Video::ControlArea::ICA1, { m_totalFrameCount + 1, 0, ica });
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
                m_cdi.m_cpu.INT1();
            break;
        }

        const uint8_t code = ica >> 24;
        switch(code)
        {
        case 0x70:
            // This command is implemented in the SCC66470, but the driver still sends it to the MCD212, so just ignore it.
            break;

        case 0xCD:
            m_renderer.SetCursorPosition(bits<0, 9>(ica), bits<12, 21>(ica));
            break;

        case 0xCE:
            m_renderer.SetCursorColor(bits<0, 3>(ica));
            m_renderer.SetCursorEnabled(bit<23>(ica));
            m_renderer.SetCursorResolution(bit<15>(ica));
            break;

        case 0xCF:
            m_renderer.SetCursorPattern(bits<16, 19>(ica), ica);
            break;

        default:
            m_renderer.ExecuteDCPInstruction<PlaneA>(ica);
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

        if(m_cdi.m_callbacks.HasOnLogICADCA())
            m_cdi.m_callbacks.OnLogICADCA(Video::ControlArea::DCA1, { m_totalFrameCount + 1, m_renderer.m_lineNumber, dca });

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
                m_cdi.m_cpu.INT1();
            break;
        }

        const uint8_t code = dca >> 24;
        switch(code)
        {
        case 0xCD:
            m_renderer.SetCursorPosition(bits<0, 9>(dca), bits<12, 21>(dca));
            break;

        case 0xCE:
            m_renderer.SetCursorColor(bits<0, 3>(dca));
            m_renderer.SetCursorEnabled(bit<23>(dca));
            m_renderer.SetCursorResolution(bit<15>(dca));
            break;

        case 0xCF:
            m_renderer.SetCursorPattern(bits<16, 19>(dca), dca);
            break;

        default:
            m_renderer.ExecuteDCPInstruction<PlaneA>(dca);
            break;
        }
    }
}

void MCD212::ExecuteICA2()
{
    const size_t cycles = GetHorizontalCycles() * GetVerticalRetraceLines();
    uint32_t addr = GetSM() && !GetPA() ? 0x200404 : 0x200400;

    for(size_t i = 0; i < cycles; i++)
    {
        const uint32_t ica = GetControlInstruction(addr);

        if(m_cdi.m_callbacks.HasOnLogICADCA())
            m_cdi.m_callbacks.OnLogICADCA(Video::ControlArea::ICA2, { m_totalFrameCount + 1, 0, ica });
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
                m_cdi.m_cpu.INT1();
            break;
        }

        // This command is implemented in the SCC66470, but the driver still sends it to the MCD212, so just ignore it.
        if(ica != 0x70000000)
            m_renderer.ExecuteDCPInstruction<PlaneB>(ica);
    }
}

void MCD212::ExecuteDCA2()
{
    for(uint8_t i = 0; i < (GetCF() ? 16 : 8); i++) // Table 5.10
    {
        const uint32_t addr = GetDCP2();
        const uint32_t dca = GetControlInstruction(addr);
        SetDCP2(addr + 4);

        if(m_cdi.m_callbacks.HasOnLogICADCA())
            m_cdi.m_callbacks.OnLogICADCA(Video::ControlArea::DCA2, { m_totalFrameCount + 1, m_renderer.m_lineNumber, dca });

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
                m_cdi.m_cpu.INT1();
            break;
        }

        m_renderer.ExecuteDCPInstruction<PlaneB>(dca);
    }
}

RAMBank MCD212::GetRAMBank1() const noexcept
{
    return {{m_memory.data(), 0x80000}, 0};
}

RAMBank MCD212::GetRAMBank2() const noexcept
{
    return {{&m_memory[0x200000], 0x80000}, 0x200000};
}
