#include "SCC68070.hpp"
#include "../../boards/Board.hpp"
#include "../../common/utils.hpp"

uint8_t SCC68070::GetPeripheral(uint32_t addr)
{
    addr -= SCC68070Peripherals::Base;

    if(addr == URHR)
    {
        internal[URHR] = board.CPUGetUART();
        LOG(disassembledInstructions.push_back("\tURHR: 0x" + toHex(internal[URHR]))) // this or data ?
    }

    return internal[addr];
}

void SCC68070::SetPeripheral(uint32_t addr, const uint8_t data)
{
    addr -= SCC68070Peripherals::Base;
    if(addr != MSR)
        internal[addr] = data;

    if(addr == UCR) // UART Command Register
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
    else if(addr == UTHR) // UART Transmit Holding Register
    {
        board.CPUSetUART(data);
        internal[USR] |= 0x08; // set TXEMT bit
        if(OnUARTOut)
            OnUARTOut(data);
        LOG(disassembledInstructions.push_back("\tUTHR: 0x" + toHex(internal[UTHR])))
    }
    else if(addr == TSR) // Timer Status Register
    {
        // Reset bits when a 1 is written in the register
        internal[TSR] ^= data;
    }
}

void SCC68070::IncrementTimer(const double ns)
{
    timerCounter += ns;
    while(timerCounter >= timerDelay)
    {
        timerCounter -= timerDelay;
        uint16_t T0 = (uint16_t)internal[T0H] << 8 | internal[T0L];

        if(T0 == 0xFFFF)
        {
            internal[TSR] |= 0x80; // If overflow, set OV flag
            internal[T0H] = internal[RRH];
            internal[T0L] = internal[RRL];
        }
        else
        {
            T0++; // increment timer
            internal[T0H] = T0 >> 8;
            internal[T0L] = T0;
        }

        if(internal[TCR] & 0x30) // T1 not inhibited
        {
            uint16_t T1 = (uint16_t)internal[T1H] << 8 | internal[T1L];

            if(T1 == 0xFFFF)
                internal[TSR] |= 0x10;
            else if((internal[TCR] & 0x30) == 0x30)
                internal[TSR] &= 0xEE; // Event-counter mode resets OV bit

            T1++;

            if(T1 == T0)
                internal[TSR] |= 0x40;

            internal[T1H] = T1 >> 8;
            internal[T1L] = T1;
        }


        if(internal[TCR] & 0x03) // T2 not inhibited
        {
            uint16_t T2 = (uint16_t)internal[T2H] << 8 | internal[T2L];

            if(T2 == 0xFFFF)
                internal[TSR] |= 0x02;
            else if((internal[TCR] & 0x03) == 0x03)
                internal[TSR] &= 0xFC; // Event-counter mode resets OV bit

            T2++;

            if(T2 == T0)
                internal[TSR] |= 0x08;

            internal[T2H] = T2 >> 8;
            internal[T2L] = T2;
        }

        // Use TCR to throw interrupts if necessary
    }
}
