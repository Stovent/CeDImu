#include "MC68HSC05C8.hpp"

/** @brief Creates a new MC68HSC05C8 MCU.
 *
 * @param internalMemory The initial memory of the MCU.
 * @param size The size of the \p internalMemory array. Should be 8KB.
 * @param outputPinCallback Called by the MCU when one of its output pin changes state or to send serial data.
 *
 * When the port is \ref Port::SPI, this means a SPI transfert has been done. The callback is called with this port by the master of the SPI,
 * the receiver of the SPI has to send its own byte back to its master during the callback. The byte is sent in the 2nd parameter (pin).
 *
 * When the port is \ref Port::SCI, this means a serial data has been send by the SCI transmitter,
 * and the value sent is in the 2nd (size_t) parameter of the callback. This value can be 8 or 9 bits depending on the configuration.
 * Reciprocally, to send data to the SCI receiver, use \ref Port::SCI as the port and put the data in the 2nd parameter.
 */
MC68HSC05C8::MC68HSC05C8(const void* internalMemory, uint16_t size, std::function<void(Port, size_t, bool)> outputPinCallback)
    : MC68HC05(memory.size())
    , memory{0}
    , SetOutputPin(outputPinCallback)
    , pendingCycles(0)
    , timerCycles(0)
    , totalCycleCount(0)
    , spiTransmit()
    , spiReceiver(0)
    , spifAccessed(false)
    , sciTransmit()
    , tdrBuffer()
    , rdrBufferRead(true)
    , tdreTcAccessed(false)
    , sciStatusAccessed(false)
    , counterLowBuffer()
    , alternateCounterLowBuffer()
    , tofAccessed(false)
    , ocfAccessed(false)
    , icfAccessed(false)
    , outputCompareInhibited(false)
    , tcapPin(false)
{
    if(internalMemory != nullptr)
    {
        if(size > memory.size())
            size = memory.size();
        memcpy(memory.data(), internalMemory, size);
    }

    Reset();
    memory[PortAData] = 0; // UUUUUUUU
    memory[PortBData] = 0; // UUUUUUUU
    memory[PortCData] = 0; // UUUUUUUU
    memory[PortDFixedInput] = 0; // UUUUUUUU
    memory[SerialPeripheralData] = 0; // UUUUUUUU
    memory[SerialCommunicationsData] = 0; // UUUUUUUU
    memory[InputCaptureHigh] = 0; // UUUUUUUU
    memory[InputCaptureLow] = 0; // UUUUUUUU
    memory[OutputCompareHigh] = 0; // UUUUUUUU
    memory[OutputCompareLow] = 0; // UUUUUUUU
}

MC68HSC05C8::~MC68HSC05C8()
{
}

void MC68HSC05C8::Reset()
{
    memory[PortADataDirection] = 0;
    memory[PortBDataDirection] = 0;
    memory[PortCDataDirection] = 0;
    memory[SerialPeripheralControl] &= 0x0F; // 00-0UUUU
    memory[SerialPeripheralStatus] = 0;
    memory[SerialCommunicationsBaudRate] &= 0x07; // --00-UUU
    memory[SerialCommunicationsControl1] &= 0xD8; // UU-UU---
    memory[SerialCommunicationsControl2] = 0;
    memory[SerialCommunicationsStatus] = 0xC0;
    memory[TimerControl] &= 0x02; // 000000U0
    memory[TimerStatus] &= 0xE0; // UUU00000
    memory[CounterHigh] = 0xFF;
    memory[CounterLow] = 0xFB;
    memory[AlternateCounterHigh] = 0xFB;
    memory[AlternateCounterLow] = 0xFB;

    pendingCycles = 0;
    timerCycles = 0;
    counterLowBuffer.reset();
    alternateCounterLowBuffer.reset();
    tofAccessed = false;
    ocfAccessed = false;
    icfAccessed = false;
    outputCompareInhibited = false;

    SetOutputPin(Port::TCMP, 0, false); // 3.14.7 TCMP pin is forced low during external reset and stays low until a valid compare changes it to a high.

    MC68HC05::Reset();
}

void MC68HSC05C8::IRQ()
{
    stop = wait = false;
    RequestInterrupt(IRQVector);
    // Here, irqPin should be changed to be set, but I am unsure for how long,
    // so I let it to its default value for simplicity.
    // In the actual hardware, it is hardwired to the CSSLAVEN signal (Chip Select Slave),
    // so probably not a problem to let it unchanged.
}

void MC68HSC05C8::IncrementTime(double ns)
{
    pendingCycles += ns / MC68HC05::INTERNAL_BUS_FREQUENCY;
    while(pendingCycles > 0)
    {
        if(!stop && !wait)
        {
            const int cycles = Interpreter();
            timerCycles += cycles;
            pendingCycles -= cycles;
            totalCycleCount += cycles;
        }
        else if(wait)
        {
            timerCycles++;
            pendingCycles--;
            totalCycleCount++;
        }
        else
            pendingCycles = 0;

        if(timerCycles >= 4)
        {
            const size_t cycles = timerCycles / 4;
            timerCycles %= 4;
            IncrementTimer(cycles);
        }

        if(spiTransmit)
            if(totalCycleCount >= spiTransmit.value().second)
            {
                SetOutputPin(Port::SPI, spiTransmit.value().first, false);
                spiTransmit.reset();

                memory[SerialPeripheralStatus] |= SPIF; // SPI Transfer complete.
                if(memory[SerialPeripheralControl] & SPIE)
                {
                    wait = false;
                    RequestInterrupt(SPIVector);
                }
            }

        if(sciTransmit)
            if(totalCycleCount >= sciTransmit.value().second)
            {
                SetOutputPin(Port::SCI, sciTransmit.value().first, false);
                sciTransmit.reset();

                bool interrupt = false;
                memory[SerialCommunicationsStatus] |= TC;
                if(memory[SerialCommunicationsControl2] & TCIE)
                    interrupt = true;

                interrupt = interrupt || LoadSCITransmitter();

                if(interrupt)
                {
                    wait = false;
                    RequestInterrupt(SCIVector);
                }
            }
    }
}

/** @brief Sets the given pin to the given state.
 *  @param port The data port.
 *  @param pin The pin number of the port (0-7).
 *  @param high true if pin is high, false if low.
 *
 * If the pin set is configured as output by the MCU, this function does nothing.
 */
void MC68HSC05C8::SetInputPin(Port port, size_t pin, bool high)
{
    const uint8_t pinMask = 1 << pin;

    switch(port)
    {
    case Port::PortA:
        if(!(memory[PortADataDirection] & pinMask)) // Input
        {
            if(high)
                memory[PortAData] |= pinMask;
            else
                memory[PortAData] &= ~pinMask;
        }
        break;

    case Port::PortB:
        if(!(memory[PortBDataDirection] & pinMask)) // Input
        {
            if(high)
                memory[PortBData] |= pinMask;
            else
                memory[PortBData] &= ~pinMask;
        }
        break;

    case Port::PortC:
        if(!(memory[PortCDataDirection] & pinMask)) // Input
        {
            if(high)
                memory[PortCData] |= pinMask;
            else
                memory[PortCData] &= ~pinMask;
        }
        break;

    case Port::PortD:
        if(pin == 7)
        {
            if(high)
                memory[PortDFixedInput] |= 0x80;
            else
                memory[PortDFixedInput] &= 0x7F;
        }
        else
        {
            if(pin == 0 && high && !(memory[SerialCommunicationsControl2] & RE)) // SCI receiver disabled
                memory[PortDFixedInput] |= 0x01;
            else
                memory[PortDFixedInput] &= 0xFE;

            if(pin == 1 && high && !(memory[SerialCommunicationsControl2] & TE)) // SCI transmitter disabled
                memory[PortDFixedInput] |= 0x02;
            else
                memory[PortDFixedInput] &= 0xFD;

            if(pin >= 2 && pin <= 5 && high && !(memory[SerialPeripheralControl] & SPE)) // SPI system disabled
                memory[PortDFixedInput] |= pinMask;
            else
                memory[PortDFixedInput] &= ~pinMask;
        }
        break;

    case Port::SPI:
        if(!(memory[SerialPeripheralControl] & MSTR)) // If Slave mode, send its byte too.
            SetOutputPin(Port::SPI, memory[SerialPeripheralData], false);
        spiReceiver = pin;
        break;

    case Port::SCI:
        if(memory[SerialCommunicationsControl2] & RE) // Receiver enabled.
        {
            if(rdrBufferRead)
            {
                memory[SerialCommunicationsData] = pin;
                rdrBufferRead = false;
                memory[SerialCommunicationsControl1] &= 0x58; // Clear R8 bit.
                if(memory[SerialCommunicationsControl1] & 0x10) // 9 bits enable.
                    memory[SerialCommunicationsControl1] |= pin >> 1 & 0x0080;

                memory[SerialCommunicationsStatus] |= RDRF;
            }
            else
                memory[SerialCommunicationsStatus] |= OR;

            if(memory[SerialCommunicationsControl2] & RIE)
            {
                wait = false;
                RequestInterrupt(SCIVector);
            }
        }
        break;

    case Port::TCAP:
        if(((memory[TimerControl] & IEDG) && !tcapPin && high) || // Rising edge.
          (!(memory[TimerControl] & IEDG) && tcapPin && !high)) // Falling edge.
        {
            memory[InputCaptureHigh] = memory[CounterHigh];
            memory[InputCaptureLow] = memory[CounterLow];

            memory[TimerStatus] |= ICF;
            if(memory[TimerControl] & ICIE && !CCR[CCRI])
            {
                wait = false;
                RequestInterrupt(TIMERVector);
            }
        }
        tcapPin = high;
        break;

    default:
        printf("[MC68HSC05C8] Wrong input pin %d", (int)port);
    }
}

uint8_t MC68HSC05C8::GetMemory(const uint16_t addr)
{
    if(addr < 0x0020)
        return GetIO(addr);

    if(addr < memory.size())
        return memory[addr];

    printf("[MC68HSC05C8] Read at 0x%X out of range\n", addr);
    return 0;
}

void MC68HSC05C8::SetMemory(const uint16_t addr, const uint8_t value)
{
    if(addr < 0x0020)
        return SetIO(addr, value);

    if(addr >= 0x0050 && addr < 0x0100)
    {
        memory[addr] = value;
        return;
    }

    printf("[MC68HSC05C8] Write at 0x%X (%d) out of range\n", addr, value);
}

uint8_t MC68HSC05C8::GetIO(uint16_t addr)
{
//    printf("[MC68HSC05C8] Get IO 0x%X\n", addr);
    switch(addr)
    {
    case SerialPeripheralStatus:
    {
        if(memory[SerialPeripheralStatus] & SPIF)
            spifAccessed = true;
        const uint8_t data = memory[SerialPeripheralStatus];
        memory[SerialPeripheralStatus] &= ~WCOL; // Clear WCOL on status read.
        return data;
    }

    case SerialPeripheralData:
        if(spifAccessed)
        {
            memory[SerialPeripheralStatus] &= ~SPIF;
            spifAccessed = false;
        }
        return spiReceiver;

    case SerialCommunicationsStatus:
        tdreTcAccessed = true;
        sciStatusAccessed = true;
        return memory[SerialCommunicationsStatus];

    case SerialCommunicationsData:
        rdrBufferRead = true;
        if(sciStatusAccessed)
        {
            sciStatusAccessed = false;
            memory[SerialCommunicationsStatus] &= TDRE | TC;
        }
        return memory[SerialCommunicationsData];

    case TimerStatus:
        if(memory[TimerStatus] & TOF) tofAccessed = true; // TOF set
        if(memory[TimerStatus] & OCF) ocfAccessed = true; // OCF set
        if(memory[TimerStatus] & ICF) icfAccessed = true; // ICF set
        return memory[TimerStatus];

    case InputCaptureLow:
        if(icfAccessed)
        {
            memory[TimerStatus] &= ~ICF; // Clear ICF
            icfAccessed = false;
        }
        return memory[InputCaptureLow];

    case OutputCompareHigh:
        outputCompareInhibited = true;
        return memory[OutputCompareHigh];

    case OutputCompareLow:
        if(ocfAccessed)
        {
            memory[TimerStatus] &= ~OCF; // Clear OCF
            ocfAccessed = false;
        }
        outputCompareInhibited = false;
        return memory[OutputCompareLow];

    case CounterHigh: // MC68HC05AG 3.14.2
    case AlternateCounterHigh:
    {
        std::optional<uint8_t>& lowBuffer = addr == CounterHigh ? counterLowBuffer : alternateCounterLowBuffer;
        if(!lowBuffer)
            lowBuffer = memory[CounterLow];
        return memory[CounterHigh];
    }
    // The timer is only in CounterHigh and CounterLow. Reading the alternate returns the counter instead.
    case CounterLow:
    case AlternateCounterLow:
    {
        if(addr == CounterLow && tofAccessed)
        {
            memory[TimerStatus] &= ~TOF; // Clear TOF
            tofAccessed = false;
        }

        std::optional<uint8_t>& lowBuffer = addr == CounterLow ? counterLowBuffer : alternateCounterLowBuffer;
        if(lowBuffer)
        {
            const uint8_t val = lowBuffer.value();
            lowBuffer.reset();
            return val;
        }
        return memory[CounterLow];
    }
    }

    return memory[addr];
}

void MC68HSC05C8::SetIO(uint16_t addr, uint8_t value)
{
    switch(addr)
    {
    case PortAData:
    case PortBData:
    case PortCData:
    {
        uint8_t diff = memory[addr] ^ value;
        for(int i = 0; i < 8; i++)
        {
            if(diff & 1 && memory[PortADataDirection + addr] & (1 << i)) // Bit changed and output.
                SetOutputPin(static_cast<Port>(addr), i, value & 1);

            diff >>= 1;
            value >>= 1;
        }
        break;
    }

    case SerialPeripheralData:
        if(memory[SerialPeripheralControl] & SPE) // Send if SPI is enabled.
        {
            if(memory[SerialPeripheralControl] & MSTR) // Master
            {
                if(spiTransmit) // Write Collision
                {
                    memory[SerialPeripheralStatus] |= WCOL;
                }
                else
                {
                    constexpr int SPR[4] = {2, 4, 16 , 32};
                    const int spr = memory[SerialPeripheralControl] & SPR01;
                    spiTransmit = {value, totalCycleCount + (8 * SPR[spr])};
                }
            }
            else // Slave
            {
                memory[SerialPeripheralData] = value;
            }
        }
        break;

    case SerialCommunicationsData:
        if(tdreTcAccessed)
        {
            tdreTcAccessed = false;
            memory[SerialCommunicationsStatus] &= ~(TDRE | TC);
        }
        if(memory[SerialCommunicationsControl2] & TE) // Only send if transmitter enable.
        {
            tdrBuffer = value;
            LoadSCITransmitter();
        }
        break;

    case PortADataDirection:
    case PortBDataDirection:
    case PortCDataDirection:
    case SerialPeripheralControl:
    case SerialCommunicationsBaudRate:
    case SerialCommunicationsControl1:
    case SerialCommunicationsControl2:
    case TimerControl:
    case OutputCompareHigh:
    case OutputCompareLow:
        memory[addr] = value;
        break;
    }

//    printf("[MC68HSC05C8] Set IO 0x%X: %d 0x%X\n", addr, value, value);
}

uint64_t MC68HSC05C8::GetSCIBaudRate() const
{
    constexpr int PRESCALER[4] = {1, 3, 4, 13};
    const int scp = memory[SerialCommunicationsBaudRate] >> 4 & 3;
    const int sciPrescaler = PRESCALER[scp];

    const int scr = memory[SerialCommunicationsBaudRate] & 7;
    const int sciRate = 1 << scr;

    return MC68HC05::INTERNAL_BUS_FREQUENCY / sciPrescaler / sciRate / 16; // Figure 3.24.
}

/** @brief Transfers the LDR buffer to the SCI shift register.
 * @return true if an interrupt has to be triggered, false otherwise.
 */
bool MC68HSC05C8::LoadSCITransmitter()
{
    if(!tdrBuffer || sciTransmit)
        return false;

    uint16_t data = tdrBuffer.value();
    tdrBuffer.reset();
    if((memory[SerialCommunicationsControl1] & 0x50) == 0x50) // 9 bits enable and 9th bit set to 1.
        data |= 0x100;

    const uint64_t baudRate = GetSCIBaudRate();
    const double cyclesPerBit = static_cast<double>(MC68HC05::INTERNAL_BUS_FREQUENCY) / static_cast<double>(baudRate);
    const uint64_t bits = memory[SerialCommunicationsControl1] & 0x10 ? 11 : 10;
    sciTransmit = {data, totalCycleCount + static_cast<uint64_t>(static_cast<double>(bits) * cyclesPerBit)};

    memory[SerialCommunicationsStatus] |= TDRE;
    if(memory[SerialCommunicationsControl2] & TIE)
        return true;
    return false;
}

void MC68HSC05C8::IncrementTimer(size_t amount)
{
    uint32_t counter = (uint32_t)memory[CounterHigh] << 8 | memory[CounterLow];
    counter += amount;
    memory[CounterHigh] = counter >> 8;
    memory[CounterLow] = counter;

    bool interrupt = false;
    if(counter > UINT16_MAX) // Timer overflow
    {
        memory[TimerStatus] |= TOF;
        if(memory[TimerControl] & TOIE && !CCR[CCRI])
            interrupt = true;
    }

    if(!outputCompareInhibited && memory[OutputCompareHigh] == memory[CounterHigh] && memory[OutputCompareLow] == memory[CounterLow])
    {
        memory[TimerStatus] |= OCF;
        if(memory[TimerControl] & OCIE && !CCR[CCRI])
            interrupt = true;
        SetOutputPin(Port::TCMP, 0, memory[TimerControl] & OLVL);
    }

    if(interrupt)
    {
        wait = false;
        RequestInterrupt(TIMERVector);
    }
}
