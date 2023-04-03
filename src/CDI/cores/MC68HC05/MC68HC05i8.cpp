#include "MC68HC05i8.hpp"

#include <algorithm>
#include <stdexcept>

#define MEMSET_RANGE(beg, endIncluded, val) memset(&memory[(beg)], (val), (endIncluded) - (beg) + 1)

/** \brief Creates a new MC68HC05i8 microcontroller.
 * \param internalMemory The initial state of the MCU's memory. Should be 16384 bytes long.
 * \param outputPinCallback The callback called by the MCU when a pin state or a peripheral changes.
 * \throw std::length_error if internalMemory is greater than 16384.
 *
 * For \p outputPinCallback the first argument is the port where the pin changed, the second argument is the pin number
 * and the third is the state of the pin (true = high, false = low).
 * See MC68HC05i8::Port for the meaning of the peripheral arguments.
 */
MC68HC05i8::MC68HC05i8(std::span<const uint8_t> internalMemory, std::function<void(Port, size_t, bool)> outputPinCallback)
    : MC68HC05(memory.size())
    , memory{0}
    , SetOutputPin(outputPinCallback)
    , pendingCycles(0)
    , timerCycles(0)
    , totalCycleCount(0)
    , coreTimer()
    , m68000Interface()
    , programmableTimer([this] (bool outputHigh) { SetOutputPin(Port::TCMP, 0, outputHigh); })
    , sci1([this] (uint16_t data) { SetOutputPin(Port::SCI1, data, false); })
    , sci2([this] (uint16_t data) { SetOutputPin(Port::SCI2, data, false); })
{
    if(internalMemory.size() > memory.size())
        throw std::length_error("Invalid MC68HC05i8's internalMemory size (must not be greater than 16384 bytes).");

    if(!internalMemory.empty())
        std::copy(internalMemory.begin(), internalMemory.end(), memory.begin());

    Reset();
    MEMSET_RANGE(PortAData, PortDData, 0);
    memory[PortEData] = 0;
    memory[PortFData] = 0;
    MEMSET_RANGE(InputCaptureAHigh, AlternateCounterLow, 0);
    MEMSET_RANGE(ChannelADataWrite, ChannelDDataRead, 0);
    // TODO:
    // 3.1.5.2 Interrupt latch
}

void MC68HC05i8::Reset()
{
    MC68HC05::Reset();

    pendingCycles = 0;
    timerCycles = 0;

    coreTimer.Reset();
    m68000Interface.Reset();
    programmableTimer.Reset();
    sci1.Reset();
    sci2.Reset();

    MEMSET_RANGE(PortADataDirection, CoreTimerCounter, 0);
    memory[PortEDataDirection] = 0;
    memory[PortFDataDirection] = 0;
    memory[PortEMode] = 0;
    MEMSET_RANGE(ChannelAStatus, ChannelDStatus, 0x11);
    MEMSET_RANGE(InterruptStatus, Mode, 0);
}

void MC68HC05i8::IRQ()
{
    stop = wait = false;
    RequestInterrupt(IRQVector);
    // Same remark as MC68HSC05C8.
}

void MC68HC05i8::IncrementTime(double ns)
{
    pendingCycles += ns / MC68HC05::INTERNAL_BUS_FREQUENCY;
    while(pendingCycles > 0)
    {
        size_t cycles = 0;

        if(!stop && !wait)
        {
            PC &= 0x3FFF; // 3.1.3 The two msb are permanently set to 0.
            if(PC < 0x0050 || (PC >= 0x0130 && PC < 0x2000)) // 4.1.3 Illegal Address Reset
                Reset();

            cycles += Interpreter();
            pendingCycles -= cycles;
            timerCycles += cycles;
            totalCycleCount += cycles;
        }
        else if(wait)
        {
            // 7.3 Timer stops during wait.
            pendingCycles--;
            totalCycleCount++;
        }
        else
            pendingCycles = 0;

        if(timerCycles >= 4)
        {
            const size_t tcycles = timerCycles / 4;
            timerCycles %= 4;

            if(programmableTimer.AdvanceCycles(tcycles))
                RequestInterrupt(TIMERVector);

            std::pair<bool, bool> intReset = coreTimer.AdvanceCycles(tcycles);
            if(intReset.second)
            {
                Reset();
                return;
            }
            else if(intReset.first)
            {
                wait = false;
                RequestInterrupt(CTIMERVector);
            }
        }

        if(sci1.AdvanceCycles(cycles))
            RequestInterrupt(SCI1Vector);
        if(sci2.AdvanceCycles(cycles))
            RequestInterrupt(SCI2Vector);
    }
}

void MC68HC05i8::SetInputPin(Port port, size_t pin, bool high)
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
        if(!(memory[PortDDataDirection] & pinMask)) // Input
        {
            if(high)
                memory[PortDData] |= pinMask;
            else
                memory[PortDData] &= ~pinMask;
        }
        break;

    case Port::PortE:
        if(pin <= 3)
        {
            const uint8_t reg = pin <= 1 ? sci1.controlRegister2 : sci2.controlRegister2;
            const uint8_t bit = pin == 0 || pin == 2 ? SCI::RE : SCI::TE;

            if(!(reg & bit)) // SCI disabled for this pin.
            {
                if(high)
                    memory[PortEData] |= pinMask;
                else
                    memory[PortEData] &= ~pinMask;
            }
        }
        else if(pin <= 7)
        {
            if(!(memory[PortEDataDirection] & pinMask) && !(memory[PortEMode] & (1 << pin))) // Input and IO pin.
            {
                if(high)
                    memory[PortEData] |= pinMask;
                else
                    memory[PortEData] &= ~pinMask;
            }
            break;
        }
        break;

    case Port::PortF:
        if(!(memory[PortFDataDirection] & pinMask) && pin < 2) // Input
        {
            if(high)
                memory[PortFData] |= pinMask;
            else
                memory[PortFData] &= ~pinMask;
        }
        break;

    case Port::SCI1:
        if(sci1.ReceiveData(pin))
            RequestInterrupt(SCI1Vector);
        break;

    case Port::SCI2:
        if(sci2.ReceiveData(pin))
            RequestInterrupt(SCI2Vector);
        break;

    default:
        printf("[MC68HC05i8] Wrong input pin %d", (int)port);
    }
}

uint8_t MC68HC05i8::GetByte(uint32_t addr)
{
    switch(addr) // Table 10-2
    {
    case 4:
        return m68000Interface.PopByteHost(M68000Interface::ChannelA);

    case 5:
        return m68000Interface.PopByteHost(M68000Interface::ChannelB);

    case 6:
        return m68000Interface.PopByteHost(M68000Interface::ChannelC);

    case 7:
        return m68000Interface.PopByteHost(M68000Interface::ChannelD);

    case 8:
        return m68000Interface.GetStatusHost(M68000Interface::ChannelA);

    case 9:
        return m68000Interface.GetStatusHost(M68000Interface::ChannelB);

    case 0xA:
        return m68000Interface.GetStatusHost(M68000Interface::ChannelC);

    case 0xB:
        return m68000Interface.GetStatusHost(M68000Interface::ChannelD);

    case 0xC:
        return m68000Interface.GetInterruptStatusHost();

    case 0xD:
        return m68000Interface.GetInterruptMaskHost();

    case 0xE:
        return m68000Interface.GetModeHost();
    }

    return 0;
}

void MC68HC05i8::SetByte(uint32_t addr, uint8_t data)
{
    switch(addr) // Table 10-2
    {
    case 0:
        if(m68000Interface.PushByteHost(M68000Interface::ChannelA, data))
            RequestInterrupt(M68KVector);
        break;

    case 1:
        if(m68000Interface.PushByteHost(M68000Interface::ChannelB, data))
            RequestInterrupt(M68KVector);
        break;

    case 2:
        if(m68000Interface.PushByteHost(M68000Interface::ChannelC, data))
            RequestInterrupt(M68KVector);
        break;

    case 3:
        if(m68000Interface.PushByteHost(M68000Interface::ChannelD, data))
            RequestInterrupt(M68KVector);
        break;

    case 0xD:
        m68000Interface.SetInterruptMaskHost(data);
        break;

    case 0xE:
        m68000Interface.SetModeHost(data);
        break;
    }
}

uint8_t MC68HC05i8::GetMemory(const uint16_t addr)
{
    if(addr < 0x0040)
        return GetIO(addr);

    if(addr < memory.size())
        return memory[addr];

    printf("[MC68HC05i8] Read at 0x%X out of range\n", addr);
    return 0;
}

void MC68HC05i8::SetMemory(const uint16_t addr, const uint8_t value)
{
    if(addr < 0x0040)
        return SetIO(addr, value);

    if(addr >= 0x0050 && addr < 0x0130)
    {
        memory[addr] = value;
        return;
    }

    if(addr == 0x3FF0 && !(value & 1)) // 8.2 Clear COP timer.
    {
        coreTimer.ClearCOP();
        return;
    }

    printf("[MC68HC05i8] Write at 0x%X (%d) out of range\n", addr, value);
}

uint8_t MC68HC05i8::GetIO(uint16_t addr)
{
    switch(addr)
    {
    case CoreTimerControlStatus:
        return coreTimer.GetControlStatusRegister();

    case CoreTimerCounter:
        return coreTimer.GetCounterRegister();

    case InputCaptureAHigh:
        return programmableTimer.GetInputCaptureAHighRegister();

    case InputCaptureALow:
        return programmableTimer.GetInputCaptureALowRegister();

    case OutputCompareHigh:
        return programmableTimer.GetOutputCompareHighRegister();

    case OutputCompareLow:
        return programmableTimer.GetOutputCompareLowRegister();

    case InputCaptureBHigh:
        return programmableTimer.GetInputCaptureBHighRegister();

    case InputCaptureBLow:
        return programmableTimer.GetInputCaptureBLowRegister();

    case CounterHigh:
        return programmableTimer.GetCounterHighRegister();

    case CounterLow:
        return programmableTimer.GetCounterLowRegister();

    case AlternateCounterHigh:
        return programmableTimer.GetAlternateCounterHighRegister();

    case AlternateCounterLow:
        return programmableTimer.GetAlternateCounterLowRegister();

    case TimerControl:
        return programmableTimer.controlRegister;

    case TimerStatus:
        return programmableTimer.GetStatusRegister();

    case SCI1Baud:
        return sci1.baudRegister;

    case SCI1Control1:
        return sci1.controlRegister1;

    case SCI1Control2:
        return sci1.controlRegister2;

    case SCI1Status:
        return sci1.GetStatusRegister();

    case SCI1Data:
        return sci1.GetDataRegister();

    case SCI2Baud:
        return sci2.baudRegister;

    case SCI2Control1:
        return sci2.controlRegister1;

    case SCI2Control2:
        return sci2.controlRegister2;

    case SCI2Status:
        return sci2.GetStatusRegister();

    case SCI2Data:
        return sci2.GetDataRegister();

    case ChannelADataRead:
        return m68000Interface.PopByteMCU(M68000Interface::ChannelA);

    case ChannelBDataRead:
        return m68000Interface.PopByteMCU(M68000Interface::ChannelB);

    case ChannelCDataRead:
        return m68000Interface.PopByteMCU(M68000Interface::ChannelC);

    case ChannelDDataRead:
        return m68000Interface.PopByteMCU(M68000Interface::ChannelD);

    case ChannelAStatus:
        return m68000Interface.GetStatusMCU(M68000Interface::ChannelA);

    case ChannelBStatus:
        return m68000Interface.GetStatusMCU(M68000Interface::ChannelB);

    case ChannelCStatus:
        return m68000Interface.GetStatusMCU(M68000Interface::ChannelC);

    case ChannelDStatus:
        return m68000Interface.GetStatusMCU(M68000Interface::ChannelD);

    case InterruptStatus:
        return m68000Interface.GetInterruptStatusMCU();

    case InterruptMask:
        return m68000Interface.GetInterruptMaskMCU();

    case Mode:
        return m68000Interface.modeMCU;
    }

    return memory[addr];
}

void MC68HC05i8::SetIO(uint16_t addr, uint8_t value)
{
    switch(addr)
    {
    case PortAData:
    case PortBData:
    case PortCData:
    case PortDData:
    case PortEData:
    case PortFData:
    {
        uint8_t dirReg;
        int regLen;
        Port port;
        if(addr == PortEData)
        {
            dirReg = memory[PortEDataDirection];
            regLen = 8;
            port = Port::PortE;
        }
        else if(addr == PortFData)
        {
            dirReg = memory[PortFDataDirection];
            regLen = 2;
            port = Port::PortF;
        }
        else
        {
            dirReg = memory[PortADataDirection + addr];
            regLen = 8;
            port = static_cast<Port>(addr);
        }

        uint8_t diff = memory[addr] ^ value;
        for(int i = 0; i < regLen; i++)
        {
            if(diff & 1 && dirReg & (1 << i)) // Bit changed and output.
                SetOutputPin(port, i, value & 1);

            diff >>= 1;
            value >>= 1;
        }
        break;
    }

    case CoreTimerControlStatus:
        return coreTimer.SetControlStatusRegister(value);

    case OutputCompareHigh:
        return programmableTimer.SetOutputCompareHighRegister(value);

    case OutputCompareLow:
        return programmableTimer.SetOutputCompareLowRegister(value);

    case TimerControl:
        programmableTimer.controlRegister = value;
        break;

    case SCI1Baud:
        sci1.baudRegister = value;
        break;

    case SCI1Control1:
        sci1.controlRegister1 = value;
        break;

    case SCI1Control2:
        sci1.controlRegister2 = value;
        break;

    case SCI1Data:
        if(sci1.SetDataRegister(value))
            RequestInterrupt(SCI1Vector);
        break;

    case SCI2Baud:
        sci2.baudRegister = value;
        break;

    case SCI2Control1:
        sci2.controlRegister1 = value;
        break;

    case SCI2Control2:
        sci2.controlRegister2 = value;
        break;

    case SCI2Data:
        if(sci2.SetDataRegister(value))
            RequestInterrupt(SCI2Vector);
        break;

    case ChannelADataWrite:
        if(m68000Interface.PushByteMCU(M68000Interface::ChannelA, value))
            SetOutputPin(Port::M68KInterface, 0, false);
        break;

    case ChannelBDataWrite:
        if(m68000Interface.PushByteMCU(M68000Interface::ChannelB, value))
            SetOutputPin(Port::M68KInterface, 0, false);
        break;

    case ChannelCDataWrite:
        if(m68000Interface.PushByteMCU(M68000Interface::ChannelC, value))
            SetOutputPin(Port::M68KInterface, 0, false);
        break;

    case ChannelDDataWrite:
        if(m68000Interface.PushByteMCU(M68000Interface::ChannelD, value))
            SetOutputPin(Port::M68KInterface, 0, false);
        break;

    case InterruptMask:
        m68000Interface.SetInterruptMaskMCU(value);
        break;

    case Mode:
        m68000Interface.modeMCU = value;
        break;

    default:
        memory[addr] = value;
    }
}
