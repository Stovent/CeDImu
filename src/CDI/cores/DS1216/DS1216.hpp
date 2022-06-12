#ifndef CDI_CORES_DS1216_DS1216_HPP
#define CDI_CORES_DS1216_DS1216_HPP

#include "../IRTC.hpp"

#include <array>
#include <deque>

class DS1216 : public IRTC
{
public:
    DS1216() = delete;
    DS1216(DS1216&) = delete;
    explicit DS1216(CDI& idc, std::time_t initialTime = 0, const uint8_t* state = nullptr);
    ~DS1216();

    void IncrementClock(const Cycles& c) override;

    uint8_t GetByte(const uint16_t addr) override;
    void SetByte(const uint16_t addr, const uint8_t data) override;

private:
    Cycles internalClock;
    std::array<uint8_t, 8> clock;
    std::array<uint8_t, 0x8000> sram; // 32KB

    void ClockToSRAM();
    void SRAMToClock();

    int patternCount; // < 0 means no match. 0 to 63 means register access.
    std::deque<bool> pattern;

    void PushPattern(const bool bit);
    void IncrementClockAccess();
};

#endif // CDI_CORES_DS1216_DS1216_HPP
