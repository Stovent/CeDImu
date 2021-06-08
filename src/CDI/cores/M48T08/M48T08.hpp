#ifndef M48T08_HPP
#define M48T08_HPP

#include <array>
#include <cstdint>
#include <ctime>

enum M48T08Registers
{
    Control = 0x1FF8,
    Seconds,
    Minutes,
    Hours,
    Day,
    Date,
    Month,
    Year,
};

struct Clock
{
    std::time_t sec;
    uint32_t nsec;
};

class M48T08
{
    Clock internalClock;
    std::array<uint8_t, 0x2000> sram;

    void ClockToSRAM();
    void SRAMToClock();

public:
    static constexpr std::time_t defaultTime = 599616000; // 1989/01/01 00:00:00

    explicit M48T08(std::time_t initialTime = 0);
    ~M48T08();

    void IncrementClock(const size_t ns);

    uint8_t GetByte(const uint16_t addr) const;
    void SetByte(const uint16_t addr, const uint8_t data);
};

#endif // M48T08_HPP
