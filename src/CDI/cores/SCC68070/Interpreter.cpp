#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>

/** \brief Exectutes a single instruction.
 * \param stopCycles The number of cycles to run if the CPU is stopped.
 * \return The number of CPU cycle executed (0 when CPU is stopped).
 * \throw SCC68070::Exception when an exception occured during exception processing.
 *
 * When the CPU is stopped, \p stopCycles is used to advance the internal timer.
 */
size_t SCC68070::SingleStep(const size_t stopCycles)
{
    std::pair<size_t, std::optional<Exception>> res = SingleStepException(stopCycles);
    if(res.second)
        PushException(*res.second);

    return res.first;
}

/** \brief Executes a single instruction and returns the exception if any.
 * \param stopCycles The number of cycles to run if the CPU is stopped.
 * \return The cycle count and the exception that occured if any.
 * \throw SCC68070::Exception when an exception occured during exception processing.
 *
 * \note A return cycle count of 0 means the CPU is stopped. If an exception occured, the non-0 vector is returned.
 * When the CPU is stopped, \p stopCycles is used to advance the internal timer (but still returns 0).
 */
std::pair<size_t, std::optional<SCC68070::Exception>> SCC68070::SingleStepException(const size_t stopCycles)
{
    size_t executionCycles = 0;
    std::optional<Exception> exception;

    if(!m_exceptions.empty())
    {
        std::priority_queue<Exception> unprocessedExceptions; // Used to store the interrupts that can't be processed because their priority is too low.

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
                const OS9::SystemCall syscall = {syscallType, m_cdi.GetBIOS().GetModuleNameAt(currentPC - m_cdi.GetBIOSBaseAddress()), inputs, ""};
                m_cdi.m_callbacks.OnLogException({ex.vector, returnAddress, exceptionVectorToString(ex.vector), syscall});
            }
//             DumpCPURegisters();
            executionCycles += ProcessException(ex.vector);
        }
        while(unprocessedExceptions.size())
        {
            m_exceptions.push(unprocessedExceptions.top());
            unprocessedExceptions.pop();
        }
    }

    if(std::find(breakpoints.cbegin(), breakpoints.cend(), PC) != breakpoints.cend())
    {
        if(!m_breakpointed)
        {
            m_breakpointed = true;
            throw Breakpoint{PC};
        }
    }

    if(m_stop)
    {
        executionCycles += stopCycles;
    }
    else
    {
        try
        {
            currentPC = PC;
            currentOpcode = GetNextWord(BUS_INSTRUCTION);
            if(m_cdi.m_callbacks.HasOnLogDisassembler())
            {
                const LogInstruction inst = {currentPC, m_cdi.GetBIOS().GetModuleNameAt(currentPC - m_cdi.GetBIOSBaseAddress()), (this->*DLUT[currentOpcode])(currentPC)};
                m_cdi.m_callbacks.OnLogDisassembler(inst);
            }
            executionCycles += (this->*ILUT[currentOpcode])();
            m_breakpointed = false;
        }
        catch(const Exception& e)
        {
            exception = e;
        }
    }

    totalCycleCount += executionCycles;

    const double ns = executionCycles * cycleDelay;
    IncrementTimer(ns);

    return std::make_pair(executionCycles, exception);
}
