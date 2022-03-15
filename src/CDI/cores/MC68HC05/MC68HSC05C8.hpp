#ifndef CDI_CORES_MC68HC05_MC68HSC05C8_HPP
#define CDI_CORES_MC68HC05_MC68HSC05C8_HPP

#include "MC68HC05.hpp"

#include <array>
#include <functional>
#include <optional>

class MC68HSC05C8 : protected MC68HC05
{
public:
    enum class Port
    {
        PortA,
        PortB,
        PortC,
        PortD,
        TCMP, /**< Timer Compare. Only as output. */
        TCAP, /**< Timer Capture. Only as input. */
    };

    MC68HSC05C8() = delete;
    MC68HSC05C8(const MC68HSC05C8&) = delete;
    MC68HSC05C8(const void* internalMemory, uint16_t size, std::function<void(Port, size_t, bool)> outputPinCallback = [] (Port, size_t, bool) {});
    ~MC68HSC05C8();

    void Reset() override;

    void IncrementTime(double ns);
    void SetInputPin(Port port, size_t pin, bool enabled);
    // TODO: IRQ.

private:
    std::array<uint8_t, 0x2000> memory;
    std::function<void(Port, size_t, bool)> SetOutputPin;
    int pendingCycles;
    int timerCycles;

    uint8_t GetMemory(uint16_t addr) override;
    void SetMemory(uint16_t addr, uint8_t value) override;
    uint8_t GetIO(uint16_t addr);
    void SetIO(uint16_t addr, uint8_t value);

    void Stop() override;
    void Wait() override;

    std::optional<uint8_t> counterLowBuffer;
    std::optional<uint8_t> alternateCounterLowBuffer;
    bool tofAccessed; // MC68HC05AG 3.14.8 cleared by accessing TSR (with TOF set) and then accessing the LSB of the free-running counter.
    bool ocfAccessed; // 3.14.8
    bool icfAccessed; // 3.14.8
    bool outputCompareInhibited;
    bool tcapPin;
    void IncrementTimer(size_t amount);

    enum IORegisters
    {
        PortAData = 0,
        PortBData,
        PortCData,
        PortDFixedInput,
        PortADataDirection,
        PortBDataDirection,
        PortCDataDirection,
        SerialPeripheralControl = 0xA,
        SerialPeripheralStatus,
        SerialPeripheralData,
        SerialCommunicationsBaudRate,
        SerialCommunicationsControl1,
        SerialCommunicationsControl2,
        SerialCommunicationsStatus,
        SerialCommunicationsData,
        TimerControl,
        TimerStatus, // Read-only.
        InputCaptureHigh, // Read-only.
        InputCaptureLow, // Read-only.
        OutputCompareHigh,
        OutputCompareLow,
        CounterHigh, // Read-only.
        CounterLow, // Read-only.
        AlternateCounterHigh, // Read-only.
        AlternateCounterLow, // Read-only.
    };

    enum IOFlags
    {
        OLVL = 0x01, // Output-Compare Level
        IEDG = 0x02, // Input-Capture Edge
        TOIE = 0x20, // Timer Overflow Interrupt Enable
        OCIE = 0x40, // Output-Compare Interrupt Enable
        ICIE = 0x80, // Input-Capture Interrupt Enable

        TOF = 0x20, // Timer Overflow Flag
        OCF = 0x40, // Output-Compare Flag
        ICF = 0x80, // Input-Capture Flag
    };
};

#endif // CDI_CORES_MC68HC05_MC68HSC05C8_HPP
