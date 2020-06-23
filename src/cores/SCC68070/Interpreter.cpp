#include "SCC68070.hpp"

#include <algorithm>
#include <chrono>
#include <iterator>

#include "../../Config.hpp"

void SCC68070::Interpreter()
{
    isRunning = true;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long double, std::nano>> start = std::chrono::steady_clock::now();
    do
    {
        if(cycleCount == 0)
        {
            LOG(std::ostream_iterator<std::string> osit(instruction, "\n"); \
                std::copy(disassembledInstructions.begin(), disassembledInstructions.end(), osit);)
            disassembledInstructions.clear();
        }

        currentPC = PC;
        currentOpcode = GetNextWord(Trigger);

        const uint16_t executionTime = (this->*ILUT[currentOpcode])();
        cycleCount += executionTime;
        totalCycleCount += executionTime;

        if(disassemble)
        {
            (this->*DLUT[currentOpcode])(currentPC);
        }

        if(!isEven(PC))
        {
            LOG(instruction << "PC NOT EVEN!" << std::endl);
            Exception(AddressError);
        }

        if(cycleCount * cycleDelay >= vdsc->GetLineDisplayTimeNanoSeconds())
        {
            vdsc->DrawLine();
            cycleCount = 0;
        }

        if(Config::limitFPS)
        {
            start += std::chrono::duration<long double, std::nano>(executionTime * cycleDelay);
            std::this_thread::sleep_until(start);
        }
//        else // Uncomment to remove delay when enabling limitFPS.
//            start = std::chrono::steady_clock::now();
    } while(loop);
    isRunning = false;
}
