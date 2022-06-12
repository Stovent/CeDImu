#include "Cycles.hpp"

#include <cassert>
#include <numeric>
#include <stdexcept>

/** \brief Constructs a new cycle counter counting at the given non-0 frequency.
 *  \throw std::invalid_argument Frequency is 0.
 */
Cycles::Cycles(uint64_t freq)
    : Cycles(freq, 0)
{
}

/** \brief Constructs a new cycle counter counting at the given non-0 frequency, starting at the given cycle count.
 *  \throw std::invalid_argument Frequency is 0.
 */
Cycles::Cycles(uint64_t freq, uint64_t cycles)
    : m_frequency(freq)
    , m_cycles(cycles)
    , m_dividend(0)
    , m_divisor(1)
{
    if(m_frequency == 0) throw std::invalid_argument("Frequency cannot be 0.");
}

/** \brief Resets the cycles counter to 0.
 */
void Cycles::Reset() noexcept
{
    m_cycles = 0;
    m_dividend = 0;
    m_divisor = 1;
}

/** \brief Increment this by based on the other cycle counter.
 * \param other The counter to add.
 *
 * This basically count the number of seconds \p other defines, counts the number of cycles elapsed by this counter
 * during the same amount of time and adds it.
 */
Cycles& Cycles::operator+=(const Cycles& other) noexcept
{
    // Compute the number of cycles executed by this during the time other executed his'.
    const uint64_t a = m_frequency * other.m_cycles;
    m_cycles += a / other.m_frequency;

    // Then to add two rational, their denominator must be the same.
    uint64_t rem = a % other.m_frequency; // remains rem / other.freq of a self cycle.
    if(m_divisor != other.m_frequency)
    {
        const uint64_t lcm = std::lcm(m_divisor, other.m_frequency);
        rem *= lcm / other.m_frequency;

        const uint64_t l = lcm / m_divisor;
        m_dividend *= l;
        m_divisor *= l;
    }

    // Finally add the new reminder to the previous one, and if a new cycle has been executed add it.
    m_dividend += rem;
    m_cycles += m_dividend / m_divisor;
    m_dividend %= m_divisor;

    return *this;
}

/** \brief Test function to validate the behavior of the cycle counter.
 */
void cyclesTests()
{
    Cycles a(7);

    Cycles b(11, 1000);
    a += b;
    assert(a.m_cycles == 636);
    assert(a.m_dividend == 4);
    assert(a.m_divisor == 11);

    Cycles c(31, 1000);
    a += c;
    assert(a.m_cycles == 862);
    assert(a.m_dividend == 58);
    assert(a.m_divisor == 341);

    Cycles d(19);

    Cycles e(13, 1000);
    d += e;
    assert(d.m_cycles == 1461);
    assert(d.m_dividend == 7);
    assert(d.m_divisor == 13);

    Cycles f(8, 1000);
    d += f;
    assert(d.m_cycles == 3836);
    assert(d.m_dividend == 56);
    assert(d.m_divisor == 104);
}
