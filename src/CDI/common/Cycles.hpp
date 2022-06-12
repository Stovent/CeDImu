#ifndef CDI_COMMON_CYCLES_HPP
#define CDI_COMMON_CYCLES_HPP

#include <cstdint>

/** \brief Counts the number of cycles elapsed by a processor.
 */
class Cycles
{
public:
    Cycles() = delete;
    Cycles(const Cycles&) = default;
    Cycles(Cycles&&) = default;
    explicit Cycles(uint64_t freq);
    Cycles(uint64_t freq, uint64_t cycles);

    Cycles& operator=(const Cycles&) = default;
    Cycles& operator=(Cycles&&) = default;
    Cycles& operator+=(const Cycles& other) noexcept;

    void Reset() noexcept;
    /** \brief Returns the number of cycles executed since the beginning of the counter. */
    operator uint64_t() const noexcept { return m_cycles; }

private:
    friend void cyclesTests();
    const uint64_t m_frequency;
    uint64_t m_cycles;

    // Remaining cycles left from previous add.
    uint64_t m_dividend;
    uint64_t m_divisor;
};

void cyclesTests();

#endif // CDI_COMMON_CYCLES_HPP
