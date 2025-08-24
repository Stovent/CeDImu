#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <chrono>

void SCC68070::Interpreter()
{
    m_isRunning = true;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::nano>> start = std::chrono::steady_clock::now();
    std::priority_queue<Exception> unprocessedExceptions; // Used to store the interrupts that can't be processed because their priority is too low.

    do
    {
        size_t executionCycles = 0;

        while(m_exceptions.size())
        {
            const Exception ex = m_exceptions.top();
            m_exceptions.pop();

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

            if(m_cdi.m_callbacks.HasOnLogException())
            {
                const uint32_t returnAddress = ex.vector == 32 || ex.vector == 45 || ex.vector == 47 ? PC + 2 : PC;
                const OS9::SystemCallType syscallType = OS9::SystemCallType(ex.vector == Trap0Instruction ? ex.data : -1);
                const std::string inputs = ex.vector == Trap0Instruction ? OS9::systemCallInputsToString(syscallType, GetCPURegisters(), [this] (const uint32_t addr) -> const uint8_t* { return this->m_cdi.GetPointer(addr); }) : "";
                const OS9::SystemCall syscall = {syscallType, m_cdi.GetModuleNameAt(currentPC), inputs, ""};
                m_cdi.m_callbacks.OnLogException({ex.vector, returnAddress, exceptionVectorToString(ex.vector), syscall});
            }
//            DumpCPURegisters();
            executionCycles += ProcessException(ex.vector);
        }
        while(unprocessedExceptions.size())
        {
            m_exceptions.push(unprocessedExceptions.top());
            unprocessedExceptions.pop();
        }

        if(m_stop)
        {
            executionCycles += 25;
        }
        else
        {
            try
            {
                currentPC = PC;
                currentOpcode = GetNextWord(BUS_INSTRUCTION);
                if(m_cdi.m_callbacks.HasOnLogDisassembler())
                {
                    const LogInstruction inst = {currentPC, m_cdi.GetModuleNameAt(currentPC), (this->*DLUT[currentOpcode])(currentPC)};
                    m_cdi.m_callbacks.OnLogDisassembler(inst);
                }
                executionCycles += (this->*ILUT[currentOpcode])();
            }
            catch(const Exception& e)
            {
                m_exceptions.push(e);
            }
        }

        totalCycleCount += executionCycles;

        const double ns = executionCycles * m_cycleDelay;
        IncrementTimer(ns);
        m_cdi.IncrementTime(ns);

        if(find(breakpoints.begin(), breakpoints.end(), currentPC) != breakpoints.end())
            m_loop = false;

        start += std::chrono::duration<double, std::nano>(executionCycles * m_speedDelay);
        std::this_thread::sleep_until(start);
    } while(m_loop);

    m_isRunning = false;
}
