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
        const SCC68070InstructionSet inst = ILUT[currentOpcode];

        uint16_t executionTime = (this->*instructions[inst])();
        cycleCount += executionTime;
        totalCycleCount += executionTime;

        if(disassemble)
        {
            (this->*Disassemble[inst])(currentPC);
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
