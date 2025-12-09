#include "SoftCDIScheduler.hpp"

#include <array>
#include <cinttypes>
#include <cstring>
#include <print>
#include <stdexcept>

/** \brief SoftCDI constructor.
 * \throw std::runtime_error if the given BIOS can't be patched.
 */
SoftCDIScheduler::SoftCDIScheduler(CDIDisc disc)
    : CDI("SoftCDIScheduler", DEFAULT_CDICONFIG, Callbacks())
    , m_cdDrive{*this, std::move(disc)}
{
    Reset(true);
}

SoftCDIScheduler::~SoftCDIScheduler()
{
    Stop(true);
}

CDIDisc& SoftCDIScheduler::GetDisc() noexcept
{
    return m_cdDrive.GetDisc();
}

void SoftCDIScheduler::Scheduler(const std::stop_token stopToken)
{
    m_isRunning = true;

    do
    {
        const SCC68070::InterpreterResult res = m_cpu.SingleStepException(25);

        const double ns = res.first * m_cpu.cycleDelay;
        IncrementTime(ns);

        if(std::holds_alternative<SCC68070::Exception>(res.second))
        {
            const SCC68070::Exception ex = std::get<SCC68070::Exception>(res.second);

            if(ex.vector == SCC68070::Trap0Instruction && ex.data >= SystemCalls::_Min) // SoftCDI syscall.
            {
                const uint16_t syscall = ex.data;

                m_cpu.GetNextWord(); // Skip syscall ID when returning.
                DispatchSystemCall(syscall);
            }
            else // CPU exception/OS-9 syscall.
            {
                m_cpu.PushException(ex);
            }
        }
        else if(std::holds_alternative<SCC68070::Breakpoint>(res.second))
        {
            break;
        }
    } while(!stopToken.stop_requested());

    m_isRunning = false;
}

void SoftCDIScheduler::IncrementTime(double ns)
{
    CDI::IncrementTime(ns);
    LocalIncrementTime(ns);
}

void SoftCDIScheduler::LocalIncrementTime(double ns)
{
    std::optional<uint8_t> irq = m_cdDrive.IncrementTime(ns);
    if(irq)
        m_cpu.PushException({static_cast<SCC68070::ExceptionVector>(*irq)});
}

void SoftCDIScheduler::Reset(const bool resetCPU)
{
    CDI::Reset(resetCPU);
    LocalReset();
}

void SoftCDIScheduler::LocalReset()
{
    m_cdDrive.Reset();
}
