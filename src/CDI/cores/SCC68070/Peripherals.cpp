#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

uint8_t SCC68070::PeekPeripheral(const uint32_t addr) const noexcept
{
    return m_peripherals.at(addr);
}

uint8_t SCC68070::GetPeripheral(uint32_t addr, const BusFlags flags)
{
    // TODO: make this Peek-safe.
    addr -= Peripheral::Base;

    std::unique_lock<std::mutex> lock(m_uartInMutex);

    if(m_uartIn.size())
        SetRXReady();
    else
        UnsetRXReady();

    if(addr == URHR)
    {
        if(m_uartIn.size())
        {
            m_peripherals[URHR] = m_uartIn.front();
            m_uartIn.pop_front();
        }
        else
            m_peripherals[URHR] = 0;

        lock.unlock();
        if(flags.log && m_cdi.m_callbacks.HasOnLogDisassembler())
            m_cdi.m_callbacks.OnLogDisassembler({currentPC, "", "URHR: 0x" + toHex(m_peripherals[URHR])}); // this or data ?
    }

    return m_peripherals[addr];
}

void SCC68070::SetPeripheral(uint32_t addr, const uint8_t data, const BusFlags flags)
{
    addr -= Peripheral::Base;

    switch(addr)
    {
    case LIR:
        // TODO: reset interrupts
        m_peripherals[LIR] = data & 0x77; // PIR is read as 0
        break;

    case UCR:
        switch(data & 0x70)
        {
        case 0x20: // reset receiver
            m_peripherals[URHR] = 0;
            break;
        case 0x30: // reset transmitter
            m_peripherals[UTHR] = 0;
            break;
        case 0x40: // Reset error status
            m_peripherals[USR] &= 0x0F;
            break;
        }

        m_peripherals[UCR] = data;
        break;

    case UTHR:
        m_peripherals[USR] |= 0x08; // set TXEMT bit
        m_cdi.m_callbacks.OnUARTOut(data);
        m_peripherals[UTHR] = data;
        if(flags.log && m_cdi.m_callbacks.HasOnLogDisassembler())
            m_cdi.m_callbacks.OnLogDisassembler({currentPC, "", "UTHR: 0x" + toHex(m_peripherals[UTHR])});
        break;

    case TSR:
        m_peripherals[TSR] ^= data; // Reset bits when a 1 is written in the register
        break;

    default:
        if(addr != MSR)
            m_peripherals[addr] = data;
    }
}

void SCC68070::IncrementTimer(const double ns)
{
    m_timerCounter += ns;
    const uint8_t priority = m_peripherals[PICR1] & 0x07;
    while(m_timerCounter >= m_timerDelay)
    {
        m_timerCounter -= m_timerDelay;
        uint16_t T0 = as<uint16_t>(m_peripherals[T0H]) << 8 | m_peripherals[T0L];

        if(T0 == 0xFFFF)
        {
            m_peripherals[TSR] |= 0x80; // If overflow, set OV flag
            m_peripherals[T0H] = m_peripherals[RRH];
            m_peripherals[T0L] = m_peripherals[RRL];
            if(priority)
                PushException(as<ExceptionVector>(Level1OnChipInterruptAutovector - 1 + priority));
        }
        else
        {
            T0++; // increment timer
            m_peripherals[T0H] = T0 >> 8;
            m_peripherals[T0L] = T0;
        }

        if(m_peripherals[TCR] & 0x30) // T1 not inhibited
        {
            uint16_t T1 = as<uint16_t>(m_peripherals[T1H]) << 8 | m_peripherals[T1L];

            if(T1 == 0xFFFF)
            {
                m_peripherals[TSR] |= 0x10;
                if(priority)
                    PushException(as<ExceptionVector>(Level1OnChipInterruptAutovector - 1 + priority));
            }
            else if((m_peripherals[TCR] & 0x30) == 0x30)
                m_peripherals[TSR] &= 0xEE; // Event-counter mode resets OV bit

            T1++;

            if(T1 == T0)
                m_peripherals[TSR] |= 0x40;

            m_peripherals[T1H] = T1 >> 8;
            m_peripherals[T1L] = T1;
        }


        if(m_peripherals[TCR] & 0x03) // T2 not inhibited
        {
            uint16_t T2 = as<uint16_t>(m_peripherals[T2H]) << 8 | m_peripherals[T2L];

            if(T2 == 0xFFFF)
            {
                m_peripherals[TSR] |= 0x02;
                if(priority)
                    PushException(as<ExceptionVector>(Level1OnChipInterruptAutovector - 1 + priority));
            }
            else if((m_peripherals[TCR] & 0x03) == 0x03)
                m_peripherals[TSR] &= 0xFC; // Event-counter mode resets OV bit

            T2++;

            if(T2 == T0)
                m_peripherals[TSR] |= 0x08;

            m_peripherals[T2H] = T2 >> 8;
            m_peripherals[T2L] = T2;
        }

        // Use TCR to throw interrupts if necessary
    }
}
