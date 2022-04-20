#ifndef CDI_CORES_MC68HC05_CORETIMER_HPP
#define CDI_CORES_MC68HC05_CORETIMER_HPP

#include <cstdint>
#include <utility>

class CoreTimer
{
public:
    enum RegistersFlags
    {
        RTIE  = 0x10,
        CTOFE = 0x20,
        RTIF  = 0x40,
        CTOF  = 0x80,
    };

    explicit CoreTimer();

    void Reset();
    [[nodiscard]] std::pair<bool, bool> AdvanceCycles(size_t cycles);
    void ClearCOP();

    [[nodiscard]] uint8_t GetCounterRegister();
    [[nodiscard]] uint8_t GetControlStatusRegister();
    void SetControlStatusRegister(uint8_t value);

private:
    uint8_t controlStatusRegister;
    uint8_t counterRegister;
    uint8_t rtiCounter;
    uint8_t copCounter;
};

#endif // CDI_CORES_MC68HC05_CORETIMER_HPP
