#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>

/** \brief Exectutes a single instruction.
 * \param stopCycles The number of cycles to run if the CPU is stopped.
 * \return The number of CPU cycle executed and what happened in the interpreter.
 * \throw SCC68070::Exception when an exception occured during exception processing.
 *
 * \note See SingleStepException for more details.
 * If an exception occured, this method pushes it to the CPU for processing and also returns it.
 * When the CPU is stopped, \p stopCycles is used to advance the internal timer.
 */
SCC68070::InterpreterResult SCC68070::SingleStep(const size_t stopCycles)
{
    InterpreterResult res = SingleStepException(stopCycles);

    if(holds_alternative<Exception>(res.second))
        PushException(std::get<Exception>(res.second));

    return res;
}

/** \brief Executes a single instruction without processing exceptions if any.
 * \param stopCycles The number of cycles to run if the CPU is stopped.
 * \return The cycle count and what happened in the interpreter.
 * \throw SCC68070::Exception when an exception occured during exception processing.
 *
 * \note If an exception occurs, it is returned by this method and will not be processed.
 *  The caller has to explicitly call `SCC68070::PushException` to process it, or use `SCC68070::SingleStep`.
 *
 * The possible returned exceptions are TRAPs, Bus errors and Address errors.
 *
 * When the CPU is stopped, \p stopCycles is used to advance the internal timer.
 * It is also added to the time necessary to process exceptions, then returned.
 */
SCC68070::InterpreterResult SCC68070::SingleStepException(const size_t stopCycles)
{
    size_t executionCycles = ProcessPendingExceptions();
    InterpreterEvent event{Normal{}};

    if(m_stop)
    {
        executionCycles += stopCycles;
    }
    else if(!m_breakpointed && std::find(m_breakpoints.cbegin(), m_breakpoints.cend(), PC) != m_breakpoints.cend())
    {
        m_breakpointed = true;
        event = Breakpoint{PC};
    }
    else
    {
        m_breakpointed = false;
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

            if(m_stop) [[unlikely]]
                event = Stopped{};
        }
        catch(const Exception& e)
        {
            event = e;
        }
    }

    totalCycleCount += executionCycles;

    const double ns = executionCycles * cycleDelay;
    IncrementTimer(ns);

    return std::make_pair(executionCycles, event);
}

/** \brief Processed the pending exceptions that are processable.
 * \return The cycle count used to process the exceptions.
 *
 * Interrupts that are too low priority compared to the IPM are left pending and will try being processed on the next call.
 */
size_t SCC68070::ProcessPendingExceptions()
{
    size_t executionCycles = 0;

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
                const OS9::SystemCall syscall{syscallType, m_cdi.GetBIOS().GetModuleNameAt(currentPC - m_cdi.GetBIOSBaseAddress()), inputs, ""};
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

    return executionCycles;
}
