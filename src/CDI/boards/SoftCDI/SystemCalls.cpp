#include "SoftCDI.hpp"

#include <print>

void SoftCDI::DispatchSystemCall(const uint16_t syscall) noexcept
{
    switch(syscall)
    {
    case CDFMDriverPlay:
        CdfmDriverPlay();
        break;

    case UCMGetStat:
        std::println("Get stat");
        break;

    case UCMSetStat:
        std::println("Set stat");
        break;

    default:
        std::println("Unknown system call 0x{:X}", syscall);
    }
}

/** \brief Handles Play routine of the CDFM device driver.
 * - a0: pointer to the destination buffer.
 * - d0.l: Logical starting sector of Play selection
 * - d1.l: Maximum number of records to play
 * - d2.l: Buffer size
 */
void SoftCDI::CdfmDriverPlay() noexcept
{
    using enum SCC68070::Register;
    std::map<SCC68070::Register, uint32_t> regs = m_cpu.GetCPURegisters();

    std::println("Play address:0x{:X} start:0x{:X} count:{} buffer size:{}", regs[A0], regs[D0], regs[D1], regs[D2]);
    m_cdDrive.StartPlaying(regs[D0], regs[D1], regs[A0]);
}
