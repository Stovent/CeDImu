#include "SCC68070.hpp"

void SCC68070::Interpreter(const bool loop)
{
    run = loop;
    do
    {
        if(cycleCount == 0)
        {
            instructionsBuffer.clear();
            if(app->mainFrame->disassemblerFrame)
                app->mainFrame->disassemblerFrame->instructions.clear();
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
            if(app->mainFrame->disassemblerFrame)
                app->mainFrame->disassemblerFrame->instructions += *--instructionsBuffer.end() + "\n";

            LOG(instruction, *--instructionsBuffer.end());
        }

        if(!isEven(PC))
        {
            LOG(instruction, "PC NOT EVEN!");
            Exception(AddressError);
        }

        instructionsBufferChanged = true;
        count++;

        if(cycleCount * cycleDelay >= vdsc->GetLineDisplayTimeNanoSeconds())
        {
            vdsc->DisplayLine();
            cycleCount = 0;
        }
    } while(run);
}
