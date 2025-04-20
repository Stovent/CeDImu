#include "SoftCDI.hpp"

#include <print>

void SoftCDI::DispatchSystemCall(const uint16_t syscall) noexcept
{
    using enum SCC68070::Register;
    std::map<SCC68070::Register, uint32_t> regs = m_cpu.GetCPURegisters();

    switch(syscall)
    {
    case CDFMDriverPlay:
        std::println("Play a1:0x{:X} a2:0x{:X} a4:0x{:X} a6:0x{:X}", regs[A1], regs[A2], regs[A4], regs[A6]);
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
