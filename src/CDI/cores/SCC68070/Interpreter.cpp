#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <chrono>

void SCC68070::Interpreter()
{
    isRunning = true;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::nano>> start = std::chrono::steady_clock::now();
    std::priority_queue<Exception> unprocessedExceptions; // Used to store the interrupts that can't be processed because their priority is too low.

    do
    {
        size_t executionCycles = 0;

        while(exceptions.size())
        {
            const Exception ex = exceptions.top();
            exceptions.pop();

            if((ex.vector >= Level1ExternalInterruptAutovector && ex.vector <= Level7ExternalInterruptAutovector) ||
               (ex.vector >= Level1OnChipInterruptAutovector && ex.vector <= Level7OnChipInterruptAutovector))
            {
                const uint8_t level = ex.vector & 0x7;
                if(level != 7 && level <= GetIPM())
                {
                    unprocessedExceptions.push(ex);
                    continue;
                }
            }

            if(cdi.m_callbacks.HasOnLogException())
            {
                const uint32_t returnAddress = ex.vector == 32 || ex.vector == 45 || ex.vector == 47 ? PC + 2 : PC;
                const OS9::SystemCallType syscallType = OS9::SystemCallType(ex.vector == Trap0Instruction ? ex.data : -1);
                const std::string inputs = ex.vector == Trap0Instruction ? OS9::systemCallInputsToString(syscallType, GetCPURegisters(), [this] (const uint32_t addr) -> const uint8_t* { return this->cdi.GetPointer(addr); }) : "";
                const OS9::SystemCall syscall = {syscallType, cdi.GetBIOS().GetModuleNameAt(currentPC - cdi.GetBIOSBaseAddress()), inputs, ""};
                cdi.m_callbacks.OnLogException({ex.vector, returnAddress, exceptionVectorToString(ex.vector), syscall});
            }
//            DumpCPURegisters();
            executionCycles += ProcessException(ex.vector);
        }
        while(unprocessedExceptions.size())
        {
            exceptions.push(unprocessedExceptions.top());
            unprocessedExceptions.pop();
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
                if(cdi.m_callbacks.HasOnLogDisassembler())
                {
                    const LogInstruction inst = {currentPC, cdi.GetBIOS().GetModuleNameAt(currentPC - cdi.GetBIOSBaseAddress()), (this->*DLUT[currentOpcode])(currentPC)};
                    cdi.m_callbacks.OnLogDisassembler(inst);
                }
                executionCycles += (this->*ILUT[currentOpcode])();
            }
            catch(const Exception& e) {
                exceptions.push(e);
            }
        }

        totalCycleCount += executionCycles;

        const double ns = executionCycles * cycleDelay;
        IncrementTimer(ns);
        cdi.IncrementTime(ns);

        if(find(breakpoints.begin(), breakpoints.end(), currentPC) != breakpoints.end())
            loop = false;

        start += std::chrono::duration<double, std::nano>(executionCycles * speedDelay);
        std::this_thread::sleep_until(start);
    } while(loop);

    isRunning = false;
}
