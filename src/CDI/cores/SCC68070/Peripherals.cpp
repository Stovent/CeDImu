#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

uint8_t SCC68070::GetPeripheral(uint32_t addr)
{
    addr -= SCC68070Peripherals::Base;
    std::unique_lock<std::mutex> lock(uartInMutex);

    if(uartIn.size())
        SET_RX_READY()
    else
        UNSET_RX_READY()

    if(addr == URHR)
    {
        if(uartIn.size())
        {
            internal[URHR] = uartIn.front();
            uartIn.pop_front();
        }
        else
            internal[URHR] = 0;

        lock.unlock();
        if(cdi.callbacks.HasOnDisassembler())
            cdi.callbacks.OnDisassembler({currentPC, "", "URHR: 0x" + toHex(internal[URHR])}); // this or data ?
    }

    return internal[addr];
}

void SCC68070::SetPeripheral(uint32_t addr, const uint8_t data)
{
    addr -= SCC68070Peripherals::Base;

    switch(addr)
    {
    case LIR:
        // TODO: reset interrupts
        internal[LIR] = data & 0x77; // PIR is read as 0
        break;

    case UCR:
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

        internal[UCR] = data;
        break;

    case UTHR:
        internal[USR] |= 0x08; // set TXEMT bit
        cdi.callbacks.OnUARTOut(data);
        internal[UTHR] = data;
        if(cdi.callbacks.HasOnDisassembler())
            cdi.callbacks.OnDisassembler({currentPC, "", "UTHR: 0x" + toHex(internal[UTHR])});
        break;

    case TSR:
        internal[TSR] ^= data; // Reset bits when a 1 is written in the register
        break;

    default:
        if(addr != MSR)
            internal[addr] = data;
    }
}

void SCC68070::IncrementTimer(const double ns)
{
    timerCounter += ns;
    const uint8_t priority = internal[PICR1] & 0x07;
    while(timerCounter >= timerDelay)
    {
        timerCounter -= timerDelay;
        uint16_t T0 = (uint16_t)internal[T0H] << 8 | internal[T0L];

        if(T0 == 0xFFFF)
        {
            internal[TSR] |= 0x80; // If overflow, set OV flag
            internal[T0H] = internal[RRH];
            internal[T0L] = internal[RRL];
            if(priority)
                Interrupt((Level1OnChipInterruptAutovector - 1) + priority, priority);
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
            {
                internal[TSR] |= 0x10;
                if(priority)
                    Interrupt((Level1OnChipInterruptAutovector - 1) + priority, priority);
            }
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
            {
                internal[TSR] |= 0x02;
                if(priority)
                    Interrupt((Level1OnChipInterruptAutovector - 1) + priority, priority);
            }
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
