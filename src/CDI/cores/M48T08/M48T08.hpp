#ifndef CDI_CORES_M48T08_M48T08_HPP
#define CDI_CORES_M48T08_M48T08_HPP

#include "../IRTC.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <optional>
#include <span>

class M48T08 : public IRTC
{
public:
    explicit M48T08(CDI& cdi, std::span<const uint8_t> state, std::optional<std::time_t> initialTime);
    ~M48T08();

    M48T08(const M48T08&) = delete;
    M48T08& operator=(const M48T08&) = delete;

    M48T08(M48T08&&) = delete;
    M48T08& operator=(M48T08&&) = delete;

    void IncrementClock(const double ns) override;

    uint8_t GetByte(const uint16_t addr) override;
    void SetByte(const uint16_t addr, const uint8_t data) override;

private:
    std::array<uint8_t, 0x2000> m_sram;
    double m_nsec; /**< Counts the nanoseconds when IncrementClock() is called. */
    std::chrono::time_point<std::chrono::system_clock> m_internalClock; /**< The SRAM internal clock. */

    void ClockToSRAM();
    void SRAMToClock();

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
};

#endif // CDI_CORES_M48T08_M48T08_HPP
