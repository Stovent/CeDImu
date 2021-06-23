#include "SCC66470.hpp"
#include "../../boards/Board.hpp"

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

        std::lock_guard<std::mutex> lock(onFrameCompletedMutex);
        if(OnFrameCompleted)
            OnFrameCompleted();
    }
}
