#ifndef CDI_CORES_MC68HC05_MC68HSC05C8_HPP
#define CDI_CORES_MC68HC05_MC68HSC05C8_HPP

#include "MC68HC05.hpp"

#include <array>
#include <functional>
#include <optional>
#include <span>

class MC68HSC05C8 : protected MC68HC05
{
public:
    enum class Port
    {
        PortA = 0,
        PortB,
        PortC,
        PortD,
        SPI, /**< SPI receive.transmit. The data is the 2nd arg (pin) of the callback. */
        SCI, /**< SCI receive/transmit. The data is the 2nd arg (pin) of the callback. */
        TCMP, /**< Timer Compare. Only as output. */
        TCAP, /**< Timer Capture. Only as input. */
    };

    MC68HSC05C8(std::span<const uint8_t> internalMemory, std::function<void(Port, size_t, bool)> outputPinCallback = [] (Port, size_t, bool) {});

    MC68HSC05C8(const MC68HSC05C8&) = default;
    MC68HSC05C8(MC68HSC05C8&&) = default;

    void Reset() override;
    void IRQ();

    void IncrementTime(double ns);
    void SetInputPin(Port port, size_t pin, bool high);

private:
    std::array<uint8_t, 0x2000> memory;
    std::function<void(Port, size_t, bool)> SetOutputPin;
    int pendingCycles;
    int timerCycles;
    uint64_t totalCycleCount; // Internal bus frequency

    uint8_t GetMemory(uint16_t addr) override;
    void SetMemory(uint16_t addr, uint8_t value) override;
    uint8_t GetIO(uint16_t addr);
    void SetIO(uint16_t addr, uint8_t value);

    // First is the data to send. Second is when totalCycleCount reaches this, actually send the data.
    std::optional<std::pair<uint8_t, uint64_t>> spiTransmit;
    uint8_t spiReceiver; // Read buffer
    bool spifAccessed;

    // First is the data to send, can be 8 or 9 bits. Second is when totalCycleCount reaches this, actually send the data.
    std::optional<std::pair<uint16_t, uint64_t>> sciTransmit;
    std::optional<uint8_t> tdrBuffer; // Transmit Data Register buffer.
    bool rdrBufferRead; // Receive Data Register buffer read since last write to it.
    bool tdreTcAccessed; // To clear TDRE and TC bits of the status register.
    bool sciStatusAccessed; // To clear the rest of the SCI status register bits.
    uint64_t GetSCIBaudRate() const;
    bool LoadSCITransmitter();

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
        SerialCommunicationsData, // Only stores the SCI receive byte.
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
        // SPI Control
        SPR01 = 0x03, // SPI Rate
        MSTR  = 0x10, // SPI Master or Slave mode select
        SPE   = 0x40, // SPI System Enable
        SPIE  = 0x80, // SPI Interrupt Enable

        // SPI Status
        WCOL = 0x40, // Write Collision
        SPIF = 0x80, // SPI Transfer Complete

        // SCI Control 2
        SBK  = 0x01, // Send Break
        RWU  = 0x02, // Receiver Wake Up
        RE   = 0x04, // Receiver Enable
        TE   = 0x08, // Transmitter Enable
        ILIE = 0x10, // IDLE Line Interrupt Enable
        RIE  = 0x20, // Receiver Interrupt Enable
        TCIE = 0x40, // Transmission Complete Interrupt Enable
        TIE  = 0x80, // Transmitter Interrupt Enable

        // SCI Status
        FE   = 0x02, // Framing Error
        NF   = 0x04, // Noise Flag
        OR   = 0x08, // Overrun
        IDLE = 0x10, // IDLE Line Detect
        RDRF = 0x20, // Receive Data Register Full
        TC   = 0x40, // Transmission Complete
        TDRE = 0x80, // Transmit Data Register Empty

        // Timer Control
        OLVL = 0x01, // Output-Compare Level
        IEDG = 0x02, // Input-Capture Edge
        TOIE = 0x20, // Timer Overflow Interrupt Enable
        OCIE = 0x40, // Output-Compare Interrupt Enable
        ICIE = 0x80, // Input-Capture Interrupt Enable

        // Timer Status
        TOF = 0x20, // Timer Overflow Flag
        OCF = 0x40, // Output-Compare Flag
        ICF = 0x80, // Input-Capture Flag
    };

    enum InterruptVectors : uint16_t
    {
        SPIVector = 0x1FF4,
        SCIVector = 0x1FF6,
        TIMERVector = 0x1FF8,
        IRQVector = 0x1FFA,
    };
};

#endif // CDI_CORES_MC68HC05_MC68HSC05C8_HPP
