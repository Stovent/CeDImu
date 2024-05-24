#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

/** \brief Exectutes a single instruction.
 * \param stopCycles The number of cycles to run if the CPU is stopped.
 * \return The number of CPU cycle executed.
 * \throw SCC68070::Exception when an exception occured during exception processing.
 *
 * A cycle count of 0 means the CPU is stopped.
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
 * A cycle count of 0 means the CPU is stopped. If no exception occured, 0 is returned, otherwise the non-0 vector.
 * When the CPU is stopped, \p stopCycles is used to advance the internal timer (but still returns 0).
 */
std::pair<size_t, std::optional<SCC68070::Exception>> SCC68070::SingleStepException(const size_t stopCycles)
{
    size_t executionCycles = 0;
    std::optional<Exception> exception;

    if(!exceptions.empty())
    {
        std::priority_queue<Exception> unprocessedExceptions; // Used to store the interrupts that can't be processed because their priority is too low.

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
//             DumpCPURegisters();
            executionCycles += ProcessException(ex.vector);
        }
        while(unprocessedExceptions.size())
        {
            exceptions.push(unprocessedExceptions.top());
            unprocessedExceptions.pop();
        }
    }

    if(stop)
    {
        executionCycles += stopCycles;
    }
    else
    {
        try
        {
            currentPC = PC;
            currentOpcode = GetNextWord(Trigger);
            if(cdi.m_callbacks.HasOnLogDisassembler())
            {
                const LogInstruction inst = {currentPC, cdi.GetBIOS().GetModuleNameAt(currentPC - cdi.GetBIOSBaseAddress()), (this->*DLUT[currentOpcode])(currentPC)};
                cdi.m_callbacks.OnLogDisassembler(inst);
            }
            executionCycles += (this->*ILUT[currentOpcode])();
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
