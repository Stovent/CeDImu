#include "SCC68070.hpp"

#include <algorithm>
#include <iterator>

void SCC68070::Interpreter(const bool loop)
{
    run = loop;
    do
    {
        if(cycleCount == 0)
        {
            LOG(std::ostream_iterator<std::string> osit(instruction, "\n"); \
                std::copy(disassembledInstructions.begin(), disassembledInstructions.end(), osit);)
            disassembledInstructions.clear();
        }

        currentPC = PC;
        currentOpcode = GetNextWord();

        uint16_t executionTime = (this->*ILUT[currentOpcode])();
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

        instructionCount++;

        if(cycleCount * cycleDelay >= vdsc->GetLineDisplayTimeNanoSeconds())
        {
            vdsc->DisplayLine();
            cycleCount = 0;
        }
    } while(run);
}
