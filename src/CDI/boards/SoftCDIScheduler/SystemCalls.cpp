#include "SoftCDIScheduler.hpp"
#include "../../cores/SCC68070/SCC68070.hpp"

#include <print>

using enum SCC68070::Register;

void SoftCDIScheduler::DispatchSystemCall(const uint16_t syscall) noexcept
{
    switch(syscall)
    {
    case SoftCDI_Debug:
        SoftCDIDebug();
        break;

    case CdDrivePlay:
        CDDrivePlay();
        break;

    case CdDriveDmaSector:
        CDDriveDmaSector();
        break;

    case CdDriveGetSubheader:
        CDDriveGetSubheader();
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

    /*
    CDFM 410CBE
    ciapdriv 42518C
    */
}

/** \brief Used to print debug info in SoftCDI. */
void SoftCDIScheduler::SoftCDIDebug() noexcept
{
    [[maybe_unused]] std::map<SCC68070::Register, uint32_t> regs = m_cpu.GetCPURegisters();

    std::println("Debug");
}

/** \brief Handles Play routine of the CDFM device driver.
 * - d0.l: Logical starting sector of Play selection.
 * - d1.l: Maximum number of records to play.
 * - d2.b: file number.
 * - d3.l: channel mask.
 */
void SoftCDIScheduler::CDDrivePlay() noexcept
{
    std::map<SCC68070::Register, uint32_t> regs = m_cpu.GetCPURegisters();

    std::println("Play start:0x{:X} count:{} file:{} channel:0x{:08X}", regs[D0], regs[D1], regs[D2], regs[D3]);
    m_cdDrive.StartPlaying(regs[D0], regs[D1], regs[D2], regs[D3]);
}

/** \brief Handles the copy of the last read sector to memory.
 * - a0: pointer to the destination buffer.
 * - d0.l: size of the destination buffer.
 */
void SoftCDIScheduler::CDDriveDmaSector() noexcept
{
    std::map<SCC68070::Register, uint32_t> regs = m_cpu.GetCPURegisters();

    std::println("DMA sector address:0x{:X} size:{}", regs[A0], regs[D0]);
    m_cdDrive.CopyLastSectorToMemory(regs[A0], regs[D0]);
}

/** \brief Returns the last read sector subheader in D0. */
void SoftCDIScheduler::CDDriveGetSubheader() noexcept
{
    const uint32_t subheader = m_cdDrive.GetLastSectorSubheader();
    std::println("Get subheader 0x{:X}", subheader);
    m_cpu.SetRegister(D0, subheader);
}
