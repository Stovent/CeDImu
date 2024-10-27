#include "SoftCDI.hpp"

#include <iostream>

void SoftCDI::DispatchSystemCall(const uint16_t syscall) noexcept
{
    switch(syscall)
    {
    case UCMGetStat:
        std::cout << "Get stat" << '\n';
        break;

    default:
        std::cerr << "Unkown system call 0x" << std::hex << syscall << '\n';
    }
}
