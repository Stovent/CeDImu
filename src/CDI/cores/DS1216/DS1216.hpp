#ifndef CDI_CORES_DS1216_DS1216_HPP
#define CDI_CORES_DS1216_DS1216_HPP

#include "../IRTC.hpp"

#include <array>
#include <chrono>
#include <deque>
#include <optional>
#include <span>

class DS1216 : public IRTC
{
public:
    explicit DS1216(CDI& cdi, std::span<const uint8_t> state, std::optional<std::time_t> initialTime);
    virtual ~DS1216() noexcept;

    DS1216(const DS1216&) = delete;
    DS1216& operator=(const DS1216&) = delete;

    DS1216(DS1216&&) = delete;
    DS1216& operator=(DS1216&&) = delete;

    virtual void IncrementClock(double ns) override;

    /** \brief Return the value at the given address without modifying the chip.
     * To access the clock registers, give an address between 0x8000 and 0x8008.
     */
    virtual uint8_t PeekByte(uint16_t addr) const noexcept override;

    virtual uint8_t GetByte(uint16_t addr, BusFlags flags) override;
    virtual void SetByte(uint16_t addr, uint8_t data, BusFlags flags) override;

private:
    std::array<uint8_t, 0x8000> m_sram; // 32KB
    std::array<uint8_t, 8> m_clock;

    double m_nsec; /**< Counts the nanoseconds when IncrementClock() is called. */
    std::chrono::time_point<std::chrono::system_clock> m_internalClock; /**< The SRAM internal clock. */
    static_assert(std::chrono::system_clock::duration::period::den >= 1000LL, "std::chrono::system_clock::duration must have millisecond precision");

    void ClockToSRAM();
    void SRAMToClock();

    int m_patternCount; // < 0 means no match. 0 to 63 means register access.
    std::deque<bool> m_pattern;

    void PushPattern(bool bit);
    void IncrementClockAccess();
};

#endif // CDI_CORES_DS1216_DS1216_HPP
