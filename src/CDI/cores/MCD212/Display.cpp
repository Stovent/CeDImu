#include "MCD212.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#define   SET_DA_BIT() m_registerCSR1R |= 0x80;
#define UNSET_DA_BIT() m_registerCSR1R &= 0x20;
#define   SET_PA_BIT() m_registerCSR1R |= 0x20;
#define UNSET_PA_BIT() m_registerCSR1R &= 0x80;

void MCD212::DrawVideoLine()
{
    if(++m_verticalLines <= GetVerticalRetraceLines())
    {
        if(m_verticalLines == 1 && GetDE()) // TODO: how to do this on every vertical line?
        {
            if(GetIC1())
                ExecuteICA1();

            if(GetIC2())
                ExecuteICA2();
        }
        return;
    }

    SET_DA_BIT()

    if(m_renderer.m_lineNumber == 0)
        m_renderer.SetPlanesResolutions(GetHorizontalResolution1(), GetHorizontalResolution2(), GetVerticalResolution());

    if(GetSM() && !isEven(m_renderer.m_lineNumber)) // not even because my line count starts at 0.
        UNSET_PA_BIT()
    else
        SET_PA_BIT()

    if(GetDE())
    {
        const uint32_t vsr1 = GetVSR1();
        const uint32_t vsr2 = GetVSR2();

        const std::pair<uint16_t, uint16_t> bytes = m_renderer.DrawLine(&m_memory[vsr1], &m_memory[vsr2]);

        SetVSR1(vsr1 + bytes.first);
        SetVSR2(vsr2 + bytes.second);

        if(GetIC1() && GetDC1())
            ExecuteDCA1();

        if(GetIC2() && GetDC2())
            ExecuteDCA2();
    }

    if(m_verticalLines >= GetTotalVerticalLines())
    {
        UNSET_DA_BIT()
        m_totalFrameCount++;
        m_verticalLines = 0;

        m_cdi.m_callbacks.OnFrameCompleted(m_renderer.RenderFrame());
    }
}
