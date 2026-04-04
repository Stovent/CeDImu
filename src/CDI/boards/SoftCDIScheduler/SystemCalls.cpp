#include "SoftCDIScheduler.hpp"
#include "../../cores/SCC68070/SCC68070.hpp"
#include "../../OS9/Stt.hpp"

#include <print>

using enum SCC68070::Register;

void SoftCDIScheduler::DispatchSystemCall(const SystemCall syscall) noexcept
{
    switch(syscall)
    {
    case SystemCall::SoftCDI_Debug:
        SoftCDIDebug();
        break;

    case SystemCall::CdDrivePlay:
        CDDrivePlay();
        break;

    case SystemCall::CdDriveCopySector:
        CDDriveCopySector();
        break;

    case SystemCall::CdDriveGetSubheader:
        CDDriveGetSubheader();
        break;

    case SystemCall::CdfmDeviceDriverGetStat:
        CDFMDeviceDriverGetStat();
        break;

    case SystemCall::CdfmDeviceDriverSetStat:
        CDFMDeviceDriverSetStat();
        break;

    case SystemCall::PointerDeviceDriverGetPacket:
        PointerDeviceDriverGetPacket();
        break;

    case SystemCall::PointerDeviceDriverGetStat:
        PointerDeviceDriverGetStat();
        break;

    case SystemCall::PointerDeviceDriverSetStat:
        PointerDeviceDriverSetStat();
        break;

    default:
        std::println("Unknown system call 0x{:X}", static_cast<uint16_t>(syscall));
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

    std::println("Debug {} {} {} {}", regs[D0], regs[D1], regs[D2], regs[D3]);
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

    // std::println("Play start:0x{:X} count:{} file:{} channel:0x{:08X}", regs[D0], regs[D1], regs[D2], regs[D3]);
    m_cdDrive.StartPlaying(regs[D0], regs[D1], regs[D2], regs[D3]);
}

/** \brief Handles the copy of the last read sector to memory.
 * - a0: pointer to the destination buffer.
 * - d0.l: size of the destination buffer.
 */
void SoftCDIScheduler::CDDriveCopySector() noexcept
{
    std::map<SCC68070::Register, uint32_t> regs = m_cpu.GetCPURegisters();

    // std::println("Copy sector address:0x{:X} size:{}", regs[A0], regs[D0]);
    m_cdDrive.CopyLastSectorToMemory(regs[A0], regs[D0]);
}

/** \brief Returns the last read sector subheader in D0. */
void SoftCDIScheduler::CDDriveGetSubheader() noexcept
{
    const uint32_t subheader = m_cdDrive.GetLastSectorSubheader();
    // std::println("Get subheader 0x{:X}", subheader);
    m_cpu.SetRegister(D0, subheader);
}

/** \brief GB VII.3.1.5.4: D0 contains the function code. */
void SoftCDIScheduler::CDFMDeviceDriverGetStat() noexcept
{
    std::map<SCC68070::Register, uint32_t> regs = m_cpu.GetCPURegisters();
    std::println("CDFM Get stat {} {} {} {} {}",
        OS9::getStatServiceRequestToString(regs[D0]),
        OS9::subGetStatServiceRequestToString(regs[D0], regs[D1]),
        regs[D2], regs[D3], regs[D4]);
}

/** \brief GB VII.3.1.5.4: D0 contains the function code. */
void SoftCDIScheduler::CDFMDeviceDriverSetStat() noexcept
{
    std::map<SCC68070::Register, uint32_t> regs = m_cpu.GetCPURegisters();
    std::println("CDFM Set stat {} {} {} {} {}",
        OS9::setStatServiceRequestToString(regs[D0]),
        OS9::subSetStatServiceRequestToString(regs[D0], regs[D1]),
        regs[D2], regs[D3], regs[D4]);
}

void SoftCDIScheduler::PointerDeviceDriverGetPacket() noexcept
{
    std::map<SCC68070::Register, uint32_t> regs = m_cpu.GetCPURegisters();
    const uint32_t a0 = regs[A0];
    std::println("Pointer get packet 0x{:X}", a0);

    m_pointerInput.GetPacket(a0);
}

void SoftCDIScheduler::PointerDeviceDriverGetStat() noexcept
{
    std::map<SCC68070::Register, uint32_t> regs = m_cpu.GetCPURegisters();
    std::println("Pointer Get stat {} {} {} {} {} {}",
        regs[D0],
        OS9::getStatServiceRequestToString(regs[D1]),
        OS9::subGetStatServiceRequestToString(regs[D1], regs[D2]),
        regs[D3], regs[D4], regs[D5]);
}

void SoftCDIScheduler::PointerDeviceDriverSetStat() noexcept
{
    std::map<SCC68070::Register, uint32_t> regs = m_cpu.GetCPURegisters();
    std::println("Pointer Set stat {} {} {} {} {} {}",
        regs[D0],
        OS9::setStatServiceRequestToString(regs[D1]),
        OS9::subSetStatServiceRequestToString(regs[D1], regs[D2]),
        regs[D3], regs[D4], regs[D5]);
}
