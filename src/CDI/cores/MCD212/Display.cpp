#include "MCD212.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

void MCD212::DrawVideoLine()
{
    if(++m_verticalLines <= GetVerticalRetraceLines())
    {
        if(m_verticalLines == 1 && GetDE()) // TODO: how to do this on every vertical line?
        {
            if(GetIC1())
                ExecuteICA<PlaneA>();

            if(GetIC2())
                ExecuteICA<PlaneB>();
        }
        return;
    }

    SetDA();

    if(GetSM())
    {
        // Odd frames displays odd lines but my line number starts at 0 instead of 1, so odd frames displays even lines.
        if(isEven(m_totalFrameCount))
        {
            UnsetPA();
            if(m_lineNumber == 0)
            {
                m_lineNumber = 1;
                m_renderer.SetDisplayFormat(GetDisplayFormat(), true);
            }
        }
        else // Odd frames, even lines.
        {
            SetPA();
            if(m_lineNumber == 0)
                m_renderer.SetDisplayFormat(GetDisplayFormat(), true);
        }
    }
    else // Non-interlaced, PA is always set.
    {
        SetPA();
        if(m_lineNumber == 0)
            m_renderer.SetDisplayFormat(GetDisplayFormat(), false);
    }

    if(GetDE())
    {
        const uint32_t vsr1 = GetVSR1();
        const uint32_t vsr2 = GetVSR2();

        const std::pair<uint16_t, uint16_t> bytes = m_renderer.DrawLine(&m_memory[vsr1], &m_memory[vsr2], m_lineNumber);

        SetVSR1(vsr1 + bytes.first);
        SetVSR2(vsr2 + bytes.second);

        if(GetIC1() && GetDC1())
            ExecuteDCA<PlaneA>();

        if(GetIC2() && GetDC2())
            ExecuteDCA<PlaneB>();
    }

    if(GetSM())
        m_lineNumber += 2;
    else
        m_lineNumber++;

    if(m_verticalLines >= GetTotalVerticalLines())
    {
        UnsetDA();
        m_totalFrameCount++;
        m_verticalLines = 0;
        m_lineNumber = 0;

        m_cdi.m_callbacks.OnFrameCompleted(m_renderer.RenderFrame());
    }
}
