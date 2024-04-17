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
        currentPC = m68000_scc68070_registers(m68000)->pc;
        m68000_exception_result_t res;
        if(cdi.m_callbacks.HasOnLogDisassembler())
        {
            std::array<char, 32> str{0};
            const m68000_disassembler_exception_result_t r = m68000_scc68070_disassembler_interpreter_exception(m68000, &m68000Callbacks, str.data(), str.size());
            res.cycles = r.cycles;
            res.exception = r.exception;
            currentPC = r.pc;

            if(res.cycles > 0)
            {
                const LogInstruction inst = {currentPC, cdi.GetBIOS().GetModuleNameAt(currentPC - cdi.GetBIOSBaseAddress()), str.data()};
                cdi.m_callbacks.OnLogDisassembler(inst);
            }
        }
        else
        {
            res = m68000_scc68070_interpreter_exception(m68000, &m68000Callbacks);
        }

        const size_t executionCycles = res.cycles == 0 && res.exception == 0 ? 25 : res.cycles; // Stop mode returns 0.
        totalCycleCount += executionCycles;

        if(res.exception != 0)
        {
            if(cdi.m_callbacks.HasOnLogException())
            {
                const uint32_t returnAddress = 0;
                const OS9::SystemCallType syscallType = OS9::SystemCallType(m68000_scc68070_peek_next_word(m68000, &m68000Callbacks).data);
                const std::string inputs = res.exception == Trap0Instruction ? OS9::systemCallInputsToString(syscallType, CPURegisters(), [this] (const uint32_t addr) -> const uint8_t* { return this->cdi.GetPointer(addr); }) : "";
                const OS9::SystemCall syscall = {syscallType, cdi.GetBIOS().GetModuleNameAt(currentPC - cdi.GetBIOSBaseAddress()), inputs, ""};
                cdi.m_callbacks.OnLogException({res.exception, returnAddress, exceptionVectorToString(res.exception), syscall});
            }

            m68000_scc68070_exception(m68000, static_cast<m68000_vector_t>(res.exception));
        }

        const double ns = executionCycles * cycleDelay;
        IncrementTimer(ns);
        cdi.IncrementTime(ns);

        start += std::chrono::duration<double, std::nano>(executionCycles * speedDelay);
        std::this_thread::sleep_until(start);
    } while(loop);

    isRunning = false;
}
