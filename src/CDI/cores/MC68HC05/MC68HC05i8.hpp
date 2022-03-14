#ifndef CDI_CORES_MC68HC05_MC68HC05I8_HPP
#define CDI_CORES_MC68HC05_MC68HC05I8_HPP

class CDI;
#include "MC68HC05.hpp"

#include <array>
#include <deque>

class MC68HC05i8 : protected MC68HC05
{
public:
    MC68HC05i8() = delete;
    MC68HC05i8(const MC68HC05i8&) = delete;
    MC68HC05i8(CDI& idc, const void* internalMemory, uint16_t size);
    ~MC68HC05i8();

    void Reset() override;

    void IncrementTime(double ns);

    uint8_t GetByte(uint32_t addr);
    void SetByte(uint32_t addr, uint8_t value);

private:
    CDI& cdi;
    std::array<uint8_t, 0x4000> memory;
    int pendingCycles;

    std::deque<uint8_t> channelReadMCU[4];
    std::deque<uint8_t> channelWriteMCU[4];
    uint8_t channelStatusMCU[4];
    uint8_t interruptStatus;
    uint8_t interruptMask;
    uint8_t modeMCU;
    uint8_t modeHost;

    uint8_t GetMemory(uint16_t addr) override;
    void SetMemory(uint16_t addr, uint8_t value) override;
    void SetIO(uint16_t addr, uint8_t value);

    void Stop() override;
    void Wait() override;

    enum IORegisters
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
};

#endif // CDI_CORES_MC68HC05_MC68HC05I8_HPP
