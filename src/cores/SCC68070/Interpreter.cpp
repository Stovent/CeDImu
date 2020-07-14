#include "SCC68070.hpp"

#include <algorithm>
#include <chrono>
#include <iterator>

#include "../../utils.hpp"

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

        if(disassemble)
        {
            disassembledInstructions.push_back((this->*DLUT[currentOpcode])(currentPC));
        }

        const uint16_t executionTime = (this->*ILUT[currentOpcode])();
        cycleCount += executionTime;
        totalCycleCount += executionTime;

        if(!isEven(PC))
        {
            disassembledInstructions.push_back("PC NOT EVEN! (" + toHex(PC) + ")");
            Exception(AddressError);
        }

        if(cycleCount * cycleDelay >= vdsc->GetLineDisplayTimeNanoSeconds())
        {
            vdsc->DrawLine();
            cycleCount = 0;
        }

        start += std::chrono::duration<long double, std::nano>(executionTime * cycleDelay);
        std::this_thread::sleep_until(start);
    } while(loop);

    isRunning = false;
}
