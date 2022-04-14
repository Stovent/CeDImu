#ifndef CDI_CORES_MC68HC05_MC68HC05I8_HPP
#define CDI_CORES_MC68HC05_MC68HC05I8_HPP

#include "MC68HC05.hpp"

#include <array>
#include <deque>
#include <functional>

class MC68HC05i8 : protected MC68HC05
{
public:
    enum class Port
    {
        PortA = 0,
        PortB,
        PortC,
        PortD,
        PortE,
        PortF,
        SCI1, /**< SCI receive/transmit 1. The data is the 2nd arg (pin) of the callback. */
        SCI2, /**< SCI receive/transmit 2. The data is the 2nd arg (pin) of the callback. */
        TCMP, /**< Timer Compare. Only as output. */
        TCAP1, /**< Timer Capture 1. Only as input. */
        TCAP2, /**< Timer Capture 2. Only as input. */
    };

    MC68HC05i8() = delete;
    MC68HC05i8(const MC68HC05i8&) = delete;
    MC68HC05i8(const void* internalMemory, uint16_t size, std::function<void(Port, size_t, bool)> outputPinCallback = [] (Port, size_t, bool) {});
    ~MC68HC05i8();

    void Reset() override;

    void IncrementTime(double ns);
    void SetInputPin(Port port, size_t pin, bool high);

private:
    std::array<uint8_t, 0x4000> memory;
    std::function<void(Port, size_t, bool)> SetOutputPin;
    int pendingCycles;
    uint64_t totalCycleCount; // Internal bus frequency

    std::deque<uint8_t> channelReadMCU[4];
    std::deque<uint8_t> channelWriteMCU[4];
    uint8_t channelStatusMCU[4];
    uint8_t interruptStatus;
    uint8_t interruptMask;
    uint8_t modeMCU;
    uint8_t modeHost;

    uint8_t GetMemory(uint16_t addr) override;
    void SetMemory(uint16_t addr, uint8_t value) override;
    uint8_t GetIO(uint16_t addr);
    void SetIO(uint16_t addr, uint8_t value);

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
        AlternateCounterHigh,
        AlternateCounterLow,
        TimerControl,
        TimerStatus = 0x1E,
        PortEMode = 0x20,
        SCI1Baud = 0x23,
        SCI1Control1,
        SCI1Control2,
        SCI1Status,
        SCI1Data,
        SCI2Baud = 0x2B,
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
