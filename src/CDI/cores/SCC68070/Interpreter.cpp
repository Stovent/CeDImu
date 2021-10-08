#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <chrono>

void SCC68070::Interpreter()
{
    isRunning = true;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::nano>> start = std::chrono::steady_clock::now();

    do
    {
        size_t executionCycles = 0;

        if(exceptions.size())
        {
            const SCC68070Exception ex = exceptions.top();
            exceptions.pop();
            if(cdi.callbacks.HasOnLogException())
            {
                const uint32_t returnAddress = ex.vector == 32 || ex.vector == 45 || ex.vector == 47 ? PC + 2 : PC;
                const OS9::SystemCallType syscallType = OS9::SystemCallType(ex.vector == Trap0Instruction ? ex.data : -1);
                const std::string inputs = ex.vector == Trap0Instruction ? OS9::systemCallInputsToString(syscallType, GetCPURegisters(), [this] (const uint32_t addr) -> const uint8_t* { return this->cdi.board->GetPointer(addr); }) : "";
                const OS9::SystemCall syscall = {syscallType, inputs, ""};
                cdi.callbacks.OnLogException({ex.vector, returnAddress, exceptionVectorToString(ex.vector), syscall});
            }
//            DumpCPURegisters();
            executionCycles += Exception(ex.vector);
        }

        if(stop)
        {
            executionCycles += 25;
        }
        else
        {
            try {
                currentPC = PC;
                currentOpcode = GetNextWord(Trigger);
                if(cdi.callbacks.HasOnLogDisassembler())
                {
                    const LogInstruction inst = {currentPC, cdi.board->GetBIOS().GetModuleNameAt(currentPC - cdi.board->GetBIOS().base), (this->*DLUT[currentOpcode])(currentPC)};
                    cdi.callbacks.OnLogDisassembler(inst);
                }
                executionCycles += (this->*ILUT[currentOpcode])();
            }
            catch(const SCC68070Exception& e) {
                exceptions.push(e);
            }
        }

        totalCycleCount += executionCycles;

        const double ns = executionCycles * cycleDelay;
        IncrementTimer(ns);
        cdi.board->IncrementTime(ns);

        if(find(breakpoints.begin(), breakpoints.end(), currentPC) != breakpoints.end())
            loop = false;

        start += std::chrono::duration<double, std::nano>(executionCycles * speedDelay);
        std::this_thread::sleep_until(start);
    } while(loop);

    isRunning = false;
}
