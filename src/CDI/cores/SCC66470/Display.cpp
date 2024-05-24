#include "SCC66470.hpp"
#include "../../CDI.hpp"

#define   SET_DA_BIT() registerCSR |= 0x80;
#define UNSET_DA_BIT() registerCSR &= 0x67;

void SCC66470::ExecuteVideoLine()
{
    SET_DA_BIT()

    if(++lineNumber >= GetVerticalResolution())
    {
        UNSET_DA_BIT()

        lineNumber = 0;
        totalFrameCount++;

        cdi.m_callbacks.OnFrameCompleted(screen);
    }
}
