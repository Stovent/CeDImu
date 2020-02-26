#include "SCC68070.hpp"

void SCC68070::Interpreter(const bool loop)
{
    run = loop;
    do
    {
        if(executionTime == 0)
        {
            instructionsBuffer.clear();
            if(app->mainFrame->disassemblerFrame)
                app->mainFrame->disassemblerFrame->instructions.clear();
        }

        currentPC = PC;
        currentOpcode = GetNextWord();
        const SCC68070InstructionSet inst = ILUT[currentOpcode];

        executionTime += (this->*instructions[inst])();
        if(disassemble)
        {
            (this->*Disassemble[inst])(currentPC);
            if(app->mainFrame->disassemblerFrame)
                app->mainFrame->disassemblerFrame->instructions += *--instructionsBuffer.end() + "\n";
#ifdef DEBUG
            instruction << *--instructionsBuffer.end() << std::endl;
#endif // DEBUG
        }

        if(!isEven(PC))
        {
#ifdef DEBUG
            instruction << "PC NOT EVEN!" << std::endl;
#endif // DEBUG
            Exception(AddressError);
        }

        instructionsBufferChanged = true;
        count++;

        if(executionTime * clockPeriod >= vdsc->GetLineDisplayTimeNanoSeconds())
        {
            vdsc->DisplayLine();
            executionTime = 0;
        }
    } while(run);
}
