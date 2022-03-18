#include "MC68HSC05C8.hpp"

/** @brief Creates a new MC68HSC05C8 MCU.
 *
 * @param internalMemory The initial memory of the MCU.
 * @param size The size of the \p internalMemory array. Should be 8KB.
 * @param outputPinCallback Called by the MCU when one of its output pin changes state.
 */
MC68HSC05C8::MC68HSC05C8(const void* internalMemory, uint16_t size, std::function<void(Port, size_t, bool)> outputPinCallback)
    : MC68HC05(memory.size())
    , memory{0}
    , SetOutputPin(outputPinCallback)
    , pendingCycles(0)
    , timerCycles(0)
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
}

MC68HSC05C8::~MC68HSC05C8()
{
}

void MC68HSC05C8::Reset()
{
    // TODO: should the undefined bits be reset anyway for determinism ?
    memory[PortADataDirection] = 0;
    memory[PortBDataDirection] = 0;
    memory[PortCDataDirection] = 0;
    memory[SerialPeripheralControl] = 0; // 00-0UUUU
    memory[SerialPeripheralStatus] = 0;
    memory[SerialPeripheralData] = 0; // UUUUUUUU
    memory[SerialCommunicationsBaudRate] = 0; // --00-UUU
    memory[SerialCommunicationsControl1] = 0; // UU-UU---
    memory[SerialCommunicationsControl2] = 0;
    memory[SerialCommunicationsStatus] = 0xC0;
    memory[SerialCommunicationsData] = 0; // UUUUUUUU
    memory[TimerControl] = 0; // 000000U0
    memory[TimerStatus] = 0; // UUU00000
    memory[InputCaptureHigh] = 0; // UUUUUUUU
    memory[InputCaptureLow] = 0; // UUUUUUUU
    memory[OutputCompareHigh] = 0; // UUUUUUUU
    memory[OutputCompareLow] = 0; // UUUUUUUU
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

void MC68HSC05C8::IncrementTime(double ns)
{
    pendingCycles += ns / MC68HC05::INTERNAL_FREQUENCY;
    while(pendingCycles > 0)
    {
        if(!stop && !wait)
        {
            const int cycles = Interpreter();
            timerCycles += cycles;
            pendingCycles -= cycles;
        }
        else if(wait)
        {
            timerCycles++;
            pendingCycles--;
        }
        else
            pendingCycles = 0;

        if(timerCycles >= 4)
        {
            const size_t cycles = timerCycles / 4;
            timerCycles %= 4;
            IncrementTimer(cycles);
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

    case Port::TCAP:
        if(((memory[TimerControl] & IEDG) && !tcapPin && high) || // Rising edge.
          (!(memory[TimerControl] & IEDG) && tcapPin && !high)) // Falling edge.
        {
            memory[InputCaptureHigh] = memory[CounterHigh];
            memory[InputCaptureLow] = memory[CounterLow];

            memory[TimerStatus] |= ICF;
            if(memory[TimerControl] & ICIE && !CCR[CCRI])
            {
                wait = false; // TODO?
                Interrupt(TIMERVector);
            }
        }
        tcapPin = high;
        break;

    default:
        printf("[MC68HSC05C8] Wrong input pin %d", (int)port);
    }
}

void MC68HSC05C8::IRQ()
{
    stop = wait = false;
    Interrupt(IRQVector);
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
    switch(addr)
    {
    case TimerStatus:
        if(memory[TimerStatus] & TOF) tofAccessed = true; // TOF set
        if(memory[TimerStatus] & OCF) ocfAccessed = true; // OCF set
        if(memory[TimerStatus] & ICF) icfAccessed = true; // ICF set
        return memory[TimerStatus];

    case InputCaptureHigh:
        return memory[InputCaptureHigh];

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

//    printf("[MC68HSC05C8] Get IO 0x%X\n", addr);
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
            if(diff & 1 && memory[PortADataDirection] & (1 << i)) // Bit changed and output.
                SetOutputPin((Port)addr, i, value & 1);

            diff >>= 1;
            value >>= 1;
        }
        break;
    }

    case PortADataDirection:
    case PortBDataDirection:
    case PortCDataDirection:
    case SerialPeripheralControl:
    case SerialPeripheralStatus:
    case SerialPeripheralData:
    case SerialCommunicationsBaudRate:
    case SerialCommunicationsControl1:
    case SerialCommunicationsControl2:
    case SerialCommunicationsStatus:
    case SerialCommunicationsData:
    case TimerControl:
    case OutputCompareHigh:
    case OutputCompareLow:
        memory[addr] = value;
        break;
    }

//    printf("[MC68HSC05C8] Set IO 0x%X: %d 0x%X\n", addr, value, value);
}

void MC68HSC05C8::Stop()
{
}

void MC68HSC05C8::Wait()
{
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
        wait = false; // TODO?
        Interrupt(TIMERVector);
    }
}
