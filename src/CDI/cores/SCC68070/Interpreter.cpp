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
        ExceptionResult res = m68000_interpreter_exception(m68000, &m68000Callbacks);
        const size_t executionCycles = res.cycles ? res.cycles : 25; // Stop mode returns 0.
        totalCycleCount += executionCycles;

        if(res.exception)
        {
            if(res.exception == Trap0Instruction)
            {
                if(cdi.m_callbacks.HasOnLogException())
                {
                    const uint32_t returnAddress = 0;
                    const OS9::SystemCallType syscallType = OS9::SystemCallType(m68000_peek_next_word(m68000, &m68000Callbacks).data);
                    const std::string inputs = res.exception == Trap0Instruction ? OS9::systemCallInputsToString(syscallType, CPURegisters(), [this] (const uint32_t addr) -> const uint8_t* { return this->cdi.GetPointer(addr); }) : "";
                    const OS9::SystemCall syscall = {syscallType, cdi.GetBIOS().GetModuleNameAt(currentPC - cdi.GetBIOSBaseAddress()), inputs, ""};
                    cdi.m_callbacks.OnLogException({res.exception, returnAddress, exceptionVectorToString(res.exception), syscall});
                }
            }

            m68000_exception(m68000, res.exception);
        }

        const double ns = executionCycles * cycleDelay;
        IncrementTimer(ns);
        cdi.IncrementTime(ns);

        start += std::chrono::duration<double, std::nano>(executionCycles * speedDelay);
        std::this_thread::sleep_until(start);
    } while(loop);

    isRunning = false;
}
