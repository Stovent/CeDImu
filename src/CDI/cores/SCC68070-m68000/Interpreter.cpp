#include "SCC68070-m68000.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <chrono>

static std::string exceptionVectorToString(const m68000_vector_t vector)
{
    switch(vector)
    {
        case ResetSspPc:  return "Reset:Initial SSP/PC";
        case AccessError:  return "Bus error";
        case AddressError:  return "Address error";
        case IllegalInstruction:  return "Illegal instruction";
        case ZeroDivide:  return "Zero divide";
        case ChkInstruction:  return "CHK instruction";
        case TrapVInstruction:  return "TRAPV instruction";
        case PrivilegeViolation:  return "Privilege violation";
        case Trace:  return "Trace";
        case LineAEmulator: return "Line A emulator";
        case LineFEmulator: return "Line F emulator";
        case FormatError: return "Format error";
        case UninitializedInterrupt: return "Uninitialized vector interrupt";
        case SpuriousInterrupt: return "Spurious interrupt";
        case Level1Interrupt: return "Level 1 interrupt autovector";
        case Level2Interrupt: return "Level 2 interrupt autovector";
        case Level3Interrupt: return "Level 3 interrupt autovector";
        case Level4Interrupt: return "Level 4 interrupt autovector";
        case Level5Interrupt: return "Level 5 interrupt autovector";
        case Level6Interrupt: return "Level 6 interrupt autovector";
        case Level7Interrupt: return "Level 7 interrupt autovector";
        case Trap0Instruction: return "TRAP 0 instruction";
        case Trap1Instruction: return "TRAP 1 instruction";
        case Trap2Instruction: return "TRAP 2 instruction";
        case Trap3Instruction: return "TRAP 3 instruction";
        case Trap4Instruction: return "TRAP 4 instruction";
        case Trap5Instruction: return "TRAP 5 instruction";
        case Trap6Instruction: return "TRAP 6 instruction";
        case Trap7Instruction: return "TRAP 7 instruction";
        case Trap8Instruction: return "TRAP 8 instruction";
        case Trap9Instruction: return "TRAP 9 instruction";
        case Trap10Instruction: return "TRAP 10 instruction";
        case Trap11Instruction: return "TRAP 11 instruction";
        case Trap12Instruction: return "TRAP 12 instruction";
        case Trap13Instruction: return "TRAP 13 instruction";
        case Trap14Instruction: return "TRAP 14 instruction";
        case Trap15Instruction: return "TRAP 15 instruction";
        case Level1OnChipInterrupt: return "Level 1 on-chip interrupt autovector";
        case Level2OnChipInterrupt: return "Level 2 on-chip interrupt autovector";
        case Level3OnChipInterrupt: return "Level 3 on-chip interrupt autovector";
        case Level4OnChipInterrupt: return "Level 4 on-chip interrupt autovector";
        case Level5OnChipInterrupt: return "Level 5 on-chip interrupt autovector";
        case Level6OnChipInterrupt: return "Level 6 on-chip interrupt autovector";
        case Level7OnChipInterrupt: return "Level 7 on-chip interrupt autovector";
        default:
            if(vector >= UserInterrupt)
                return "User interrupt vector " + std::to_string(vector - UserInterrupt);
            return "Unknown vector " + std::to_string(vector);
    }
}

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

    if(std::holds_alternative<Exception>(res.second))
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
    size_t executionCycles = 0;
    InterpreterEvent event{Normal{}};

    currentPC = m_regs->pc;

    if(!m_breakpointed && std::find(m_breakpoints.cbegin(), m_breakpoints.cend(), currentPC) != m_breakpoints.cend())
    {
        m_breakpointed = true;
        event = Breakpoint{currentPC};
    }
    else
    {
        m_breakpointed = false;

        m68000_exception_result_t result;
        if(m_cdi.m_callbacks.HasOnLogDisassembler())
        {
            char inst[64];
            const m68000_disassembler_exception_result_t res = m68000_scc68070_disassembler_interpreter_exception(m_m68000.get(), &m_memory, inst, sizeof inst);
            if(res.cycles != 0) // CPU stopped: don't log anything.
            {
                const LogInstruction logInst = {res.pc, m_cdi.GetBIOS().GetModuleNameAt(res.pc - m_cdi.GetBIOSBaseAddress()), std::string(inst)};
                m_cdi.m_callbacks.OnLogDisassembler(logInst);
            }
            result.cycles = res.cycles;
            result.exception = res.exception;
        }
        else
        {
            result = m68000_scc68070_interpreter_exception(m_m68000.get(), &m_memory);
        }

        // TODO: this is wrong. we detect stopped state one instruction too late.
        if(result.cycles == 0) [[unlikely]]
            event = Stopped{};
        executionCycles = result.cycles != 0 ? result.cycles : stopCycles; // 0 means CPU is stopped;
        const m68000_vector_t ex = static_cast<m68000_vector_t>(result.exception);

        if(ex)
        {
            if(m_cdi.m_callbacks.HasOnLogException())
            {
                const uint32_t returnAddress = ex == Trap0Instruction || ex == Trap13Instruction || ex == Trap15Instruction ? m_regs->pc + 2 : m_regs->pc;
                const uint16_t data = ex == Trap0Instruction ? OS9::SystemCallType(m68000_scc68070_peek_next_word(m_m68000.get(), &m_memory).data) : -1;
                const OS9::SystemCallType syscallType = OS9::SystemCallType(data);
                const std::string inputs = ex == Trap0Instruction ? OS9::systemCallInputsToString(syscallType, GetCPURegisters(), [this] (const uint32_t addr) -> const uint8_t* { return this->m_cdi.GetPointer(addr).data(); }) : "";
                const OS9::SystemCall syscall = {syscallType, m_cdi.GetBIOS().GetModuleNameAt(currentPC - m_cdi.GetBIOSBaseAddress()), inputs, ""};
                m_cdi.m_callbacks.OnLogException({result.exception, returnAddress, exceptionVectorToString(ex), syscall});
            }
            event = Exception{ex, PeekNextWord()};
        }
    }

    totalCycleCount += executionCycles;

    const double ns = executionCycles * cycleDelay;
    IncrementTimer(ns);

    return std::make_pair(executionCycles, event);
}
