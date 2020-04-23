#include "SCC68070.hpp"

uint8_t SCC68070::GetPeripheral(const uint32_t addr)
{
    const uint8_t data = internal[addr - SCC68070Peripherals::Base];

    if(addr == 0x8000201B)
    {
        internal[URHR] = ReadUART();
    }

    return data;
}

void SCC68070::SetPeripheral(const uint32_t addr, const uint8_t data)
{
    if(addr == 0x80002017) // UART Command Register
    {
        switch(data & 0x70)
        {
        case 0x20: // reset receiver
            internal[URHR] = 0;
            break;
        case 0x30: // reset transmitter
            internal[UTHR] = 0;
            break;
        case 0x40: // Reset error status
            internal[USR] &= 0x0F;
            break;
        }
    }
    else if(addr == 0x80002019) // UART Transmit Holding Register
    {
        WriteUART(data);
    }

    internal[addr - SCC68070Peripherals::Base] = data;
}

uint8_t SCC68070::ReadUART()
{
    int c = uart_in.get();
    LOG(out << std::hex << currentPC << "\tURHR: 0x" << c << std::endl)
    return (c != EOF) ? c : 0;
}

void SCC68070::WriteUART(const uint8_t data)
{
    internal[UTHR] = data;
    LOG(uart_out.write((char*)&data, 1))
    LOG(out << std::hex << currentPC << "\tUTHR: 0x" << (uint32_t)internal[UTHR] << std::endl)
    internal[USR] |= 0x08; // set TXEMT bit
}
