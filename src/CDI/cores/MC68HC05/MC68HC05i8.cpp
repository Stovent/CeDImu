#include "MC68HC05i8.hpp"
#include "../../CDI.hpp"

#include <cstring>

#define SLAVE_FREQUENCY (4'000'000)

#define MEMSET_RANGE(beg, endIncluded, val) memset(&memory[beg], val, endIncluded - beg + 1)
#define NIBBLE_SWAP(val) (uint8_t)(val >> 4 | val << 4)

enum MC68HC05i8IORegisters
{
    PortAData = 0,
    PortBData,
    PortCData,
    PortDData,
    PortADataDirection,
    PortBDataDirection,
    PortCDataDirection,
    PortDDataDirection,
    CoreTimerControlStatus,
    CoreTimerCounter,
    PortEData = 0xC,
    PortEDataDirection,
    PortFData,
    PortFDataDirection,
    InputCaptureAHigh,
    InputCaptureALow,
    OutputCompareAHigh,
    OutputCompareALow,
    InputCaptureBHigh,
    InputCaptureBLow,
    CountHigh = 0x18,
    CountLow,
    AlternateCountHigh,
    AlternateCountLow,
    TimerControl,
    TimerStatus = 0x1E,
    PortEMode = 0x20,
    SCI1Baud= 0x23,
    SCI1Control1,
    SCI1Control2,
    SCI1Status,
    SCI1Data,
    SCI2Baud= 0x2B,
    SCI2Control1,
    SCI2Control2,
    SCI2Status,
    SCI2Data,
    ChannelADataWrite,
    ChannelBDataWrite,
    ChannelCDataWrite,
    ChannelDDataWrite,
    ChannelADataRead,
    ChannelBDataRead,
    ChannelCDataRead,
    ChannelDDataRead,
    ChannelAStatus,
    ChannelBStatus,
    ChannelCStatus,
    ChannelDStatus,
    InterruptStatus,
    InterruptMask,
    Mode,
};

MC68HC05i8::MC68HC05i8(CDI& idc, const void* internalMemory, uint16_t size)
    : MC68HC05(memory.size())
    , cdi(idc)
    , memory{0}
    , pendingCycles(0)
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
    // TODO:
    // 3.1.5.2 Interrupt latch
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
    if(!waitStop)
    {
        pendingCycles += ns / SLAVE_FREQUENCY;
        while(pendingCycles > 0)
        {
            PC &= 0x3FFF; // 3.1.3 The two msb are permanently set to 0.
            if(PC < 0x0050 || (PC >= 0x0130 && PC < 0x2000)) // 4.1.3 Illegal Address Reset
                Reset();
            pendingCycles -= Interpreter();
        }
    }
}

uint8_t MC68HC05i8::GetByte(uint32_t addr)
{
    return 0;
}

void MC68HC05i8::SetByte(uint32_t addr, uint8_t value)
{

}

uint8_t MC68HC05i8::GetMemory(const uint16_t addr)
{
    if(addr < memory.size())
        return memory[addr]; // TODO: GetIO(addr) ?

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

void MC68HC05i8::SetIO(uint16_t addr, uint8_t value)
{

}

void MC68HC05i8::Stop()
{

}

void MC68HC05i8::Wait()
{

}
