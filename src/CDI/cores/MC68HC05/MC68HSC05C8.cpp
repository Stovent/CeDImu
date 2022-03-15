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
}

MC68HSC05C8::~MC68HSC05C8()
{
}

void MC68HSC05C8::Reset()
{
    // TODO: should the undefined bits be reset anyway for determinism ?
    memory[PortAData] = 0; // UUUUUUUU
    memory[PortBData] = 0; // UUUUUUUU
    memory[PortCData] = 0; // UUUUUUUU
    memory[PortDFixedInput] = 0; // UUUUUUUU
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
        if(!waitStop)
        {
            const int cycles = Interpreter();
            timerCycles += cycles;
            pendingCycles -= cycles;
        }

        // TODO: STOP and WAIT behaviour on timer.
        if(timerCycles >= 4)
        {
            const size_t cycles = timerCycles / 4;
            timerCycles %= 4;
            IncrementTimer(cycles);
        }
    }
}

void MC68HSC05C8::SetInputPin(Port port, size_t pin, bool enabled)
{
    switch(port)
    {
    case Port::TCAP:
        if(((memory[TimerControl] & IEDG) && !tcapPin && enabled) || // Rising edge.
          (!(memory[TimerControl] & IEDG) && tcapPin && !enabled)) // Falling edge.
        {
            memory[InputCaptureHigh] = memory[CounterHigh];
            memory[InputCaptureLow] = memory[CounterLow];

            memory[TimerStatus] |= ICF;
            if(memory[TimerControl] & ICIE && !CCR[CCRI])
                ; // TODO: correctly generate interrupt.
        }
        tcapPin = enabled;
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

    return memory[addr];
}

void MC68HSC05C8::SetIO(uint16_t addr, uint8_t value)
{
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
        PC = (uint16_t)memory[0x1FF8] << 8 | memory[0x1FF9];
        // TODO: Interrupt method to wake up the CPU in case of WAIT instruction.
    }
}
