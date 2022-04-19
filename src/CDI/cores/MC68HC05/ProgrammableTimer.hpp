#ifndef CDI_CORES_MC68HC05_PROGRAMMABLETIMER_HPP
#define CDI_CORES_MC68HC05_PROGRAMMABLETIMER_HPP

#include <cstdint>
#include <functional>
#include <optional>

class ProgrammableTimer
{
public:
    enum RegistersFlags
    {
        // Control
        OLVL  = 0x01,
        IEDGB = 0x02,
        IEDGA = 0x04,
        TOIE  = 0x10,
        OCIE  = 0x20,
        ICBIE = 0x40,
        ICAIE = 0x80,

        // Status
        TOF  = 0x10,
        OCF  = 0x20,
        ICBF = 0x40,
        ICAF = 0x80,
    };

    uint8_t controlRegister;

    ProgrammableTimer() = delete;
    explicit ProgrammableTimer(std::function<void(bool)> outputCompareCallback);

    void Reset();
    bool AdvanceCycles(size_t cycles);

    bool SetTCAPA(bool high);
    bool SetTCAPB(bool high);

    uint8_t GetCounterHighRegister();
    uint8_t GetCounterLowRegister();
    uint8_t GetAlternateCounterHighRegister();
    uint8_t GetAlternateCounterLowRegister();
    uint8_t GetInputCaptureAHighRegister();
    uint8_t GetInputCaptureALowRegister();
    uint8_t GetInputCaptureBHighRegister();
    uint8_t GetInputCaptureBLowRegister();
    uint8_t GetOutputCompareHighRegister();
    uint8_t GetOutputCompareLowRegister();
    uint8_t GetStatusRegister();

    void SetOutputCompareHighRegister(uint8_t value);
    void SetOutputCompareLowRegister(uint8_t value);

private:
    uint8_t counterHigh;
    uint8_t counterLow;
    uint8_t inputCaptureAHigh;
    uint8_t inputCaptureALow;
    uint8_t inputCaptureBHigh;
    uint8_t inputCaptureBLow;
    uint8_t outputCompareHigh;
    uint8_t outputCompareLow;
    uint8_t statusRegister;

    bool tofAccessed; // 7.2.2 To clear the status flags.
    bool ocfAccessed;
    bool icafAccessed;
    bool icbfAccessed;

    bool tcapPinA;
    bool tcapPinB;
    bool inputCaptureAInhibited;
    bool inputCaptureBInhibited;

    std::function<void(bool)> OutputCompareCallback;
    bool outputCompareInhibited;
    std::optional<uint8_t> counterLowBuffer;
    std::optional<uint8_t> alternateCounterLowBuffer;
};

#endif // CDI_CORES_MC68HC05_PROGRAMMABLETIMER_HPP

