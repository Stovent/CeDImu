#include "MC68HC05i8.hpp"

#include <cstring>

#define MEMSET_RANGE(beg, endIncluded, val) memset(&memory[beg], val, endIncluded - beg + 1)
#define NIBBLE_SWAP(val) (uint8_t)(val >> 4 | val << 4)

MC68HC05i8::MC68HC05i8(const void* internalMemory, uint16_t size, std::function<void(Port, size_t, bool)> outputPinCallback)
    : MC68HC05(memory.size())
    , memory{0}
    , SetOutputPin(outputPinCallback)
    , pendingCycles(0)
    , totalCycleCount(0)
    , channelReadMCU{{0}}
    , channelWriteMCU{{0}}
    , channelStatusMCU{0} // TODO: init status
    , interruptStatus(0)
    , interruptMask(0)
    , modeMCU(0)
    , modeHost(0)
{
    if(internalMemory != nullptr)
    {
        if(size > memory.size())
            size = memory.size();
        memcpy(memory.data(), internalMemory, size);
    }

    Reset();
    MEMSET_RANGE(PortAData, PortDData, 0);
    memory[PortEData] = 0;
    memory[PortFData] = 0;
    MEMSET_RANGE(InputCaptureAHigh, AlternateCounterLow, 0);
    MEMSET_RANGE(ChannelADataWrite, ChannelDDataRead, 0);
    // TODO:
    // 3.1.5.2 Interrupt latch
}

MC68HC05i8::~MC68HC05i8()
{
}

void MC68HC05i8::Reset()
{
    MC68HC05::Reset();

    pendingCycles = 0;

    MEMSET_RANGE(PortADataDirection, CoreTimerCounter, 0);
    memory[CoreTimerControlStatus] = 0x03;
    memory[PortEDataDirection] = 0;
    memory[PortFDataDirection] = 0;
    memory[TimerControl] &= 0x06;
    memory[TimerStatus] &= 0xE0;
    memory[PortEMode] = 0;
    memory[SCI1Baud] &= 0x07;
    memory[SCI1Control1] &= 0xC0;
    memory[SCI1Control2] = 0;
    memory[SCI1Status] = 0xC0;
    memory[SCI1Data] = 0;
    memory[SCI2Baud] &= 0x07;
    memory[SCI2Control1] &= 0xC0;
    memory[SCI2Control2] = 0;
    memory[SCI2Status] = 0xC0;
    memory[SCI2Data] = 0;
    MEMSET_RANGE(ChannelAStatus, ChannelDStatus, 0x11);
    MEMSET_RANGE(InterruptStatus, Mode, 0);
}

void MC68HC05i8::IncrementTime(double ns)
{
    pendingCycles += ns / MC68HC05::INTERNAL_BUS_FREQUENCY;
    while(pendingCycles > 0)
    {
        if(!stop && !wait)
        {
            const int cycles = Interpreter();
            pendingCycles -= cycles;
            totalCycleCount += cycles;
        }
        else if(wait)
        {
            pendingCycles--;
            totalCycleCount++;
        }
        else
            pendingCycles = 0;
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
            const uint8_t reg = pin <= 1 ? memory[SCI1Control2] : memory[SCI2Control2];
            const uint8_t bit = pin == 0 || pin == 2 ? RE : TE;

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

    default:
        printf("[MC68HC05i8] Wrong input pin %d", (int)port);
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

    printf("[MC68HC05i8] Write at 0x%X (%d) out of range\n", addr, value);
}

uint8_t MC68HC05i8::GetIO(uint16_t addr)
{
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
    }
}
