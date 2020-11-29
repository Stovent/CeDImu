#include "SCC68070.hpp"
#include "../../utils.hpp"
#include "../../Boards/Board.hpp"

uint8_t SCC68070::GetPeripheral(const uint32_t addr)
{
    if(addr == 0x8000201B)
    {
        internal[URHR] = board->CPUGetUART();
        LOG(disassembledInstructions.push_back("\tURHR: 0x" + toHex(internal[URHR]))) // this or data ?
    }

    return internal[addr - SCC68070Peripherals::Base];
}

void SCC68070::SetPeripheral(const uint32_t addr, const uint8_t data)
{
    internal[addr - SCC68070Peripherals::Base] = data;

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

        if(data & 0x01)
            SET_RX_READY();
        if(data & 0x02)
            UNSET_RX_READY();
        if(data & 0x04)
            SET_TX_READY();
        if(data & 0x08)
            UNSET_TX_READY();
    }
    else if(addr == 0x80002019) // UART Transmit Holding Register
    {
        board->CPUSetUART(data);
        internal[USR] |= 0x08; // set TXEMT bit
        LOG(disassembledInstructions.push_back("\tUTHR: 0x" + toHex(internal[UTHR])))
    }
}
