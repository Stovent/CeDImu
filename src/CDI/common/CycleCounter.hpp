#ifndef CDI_COMMON_CYCLECOUNTER_HPP
#define CDI_COMMON_CYCLECOUNTER_HPP

#include <chrono>
#include <numeric>
#include <print>

/** \brief Type that stores the cycle count. */
using CycleRep = uint64_t;

/** \brief Stores the cycle count of a chip.
 * \tparam FREQ The frequency of the chip.
 * \tparam REFERENCE_DURATION The duration type that will be added to this type.
 */
template<CycleRep FREQ, typename REFERENCE_DURATION>
class CycleCounter
{
public:
    static constexpr CycleRep FREQUENCY = FREQ;
    using Period = std::ratio<1, FREQUENCY>;
    using Duration = std::chrono::duration<CycleRep, Period>;
    using ReferenceDuration = REFERENCE_DURATION;

    constexpr CycleCounter(const CycleRep& cycles) : CycleCounter{Duration{cycles}} {}
    constexpr CycleCounter(const Duration& cycles) : m_cycles{cycles} {}

//     /** \brief Increment the cycle count by the given amount of time.
//      * TODO: return the number of cycles this incremented by.
//      */
//     constexpr void IncrementBy(SubCycleDuration deltaDuration)
//     {
//         m_subCycle += deltaDuration;
//
//         while(m_subCycle >= FULL_SUBCYCLE)
//         {
//             m_subCycle -= FULL_SUBCYCLE;
//             ++m_cycles;
//         }
//     }

    /** \brief Increment the cycle count up to the given time. */
    constexpr CycleRep IncrementTo(const ReferenceDuration& targetDuration) noexcept
    {
        constexpr Duration target = std::chrono::duration_cast(targetDuration);
        constexpr CycleRep delta = target - m_cycles;

        m_cycles = target;
        return delta;
    }

    /** \brief Increment the cycle count up to the given time. */
    [[nodiscard]] constexpr CycleCounter IncrementTo(const ReferenceDuration& targetDuration) const noexcept
    {
        return CycleCounter{std::chrono::duration_cast<Duration>(targetDuration)};
    }

private:
    friend constexpr int testCycleCounter();
    Duration m_cycles{};
};

/** \brief Fires events when the given amount of cycles is reached.
 * \tparam FREQ The frequency at which the cycle count increments.
 * The delay can be changed at runtime.
 */
template<CycleRep FREQ>
class CycleTimer
{
    static constexpr CycleRep FREQUENCY{FREQ};
    using Period = std::ratio<1, FREQUENCY>;
    using Duration = std::chrono::duration<CycleRep, Period>;

    /** \brief Creates a CycleTimer which indicates events on the given cycle count.
     * \param delay The number of cycles between each event.
     */
    explicit constexpr CycleTimer(const CycleRep delay) noexcept : m_delay{Duration{delay}} {}

    [[nodiscard]] constexpr Duration GetLastEvent() const noexcept { return m_lastEvent; }
    [[nodiscard]] constexpr CycleRep GetDelay() const noexcept { return m_delay; }
    constexpr void SetDelay(const CycleRep delay) noexcept { m_delay = delay; }

    /** \brief Compares the last event to the given time and returns whether the timer reached the required time. */
    template<typename OTHER_DURATION>
    constexpr bool IncrementTo(const OTHER_DURATION& target) noexcept
    {
        const Duration cycles = std::chrono::duration_cast<Duration>(target);

        if(cycles - m_lastEvent < m_delay) // does not allow multiple events at a time.
            return false;

        m_lastEvent += m_delay;
        return true;
    }

private:
    Duration m_lastEvent{}; /**< The time at which the last event occured. */
    Duration m_delay{}; /**< The time delay between two events. */

    friend constexpr int testCycleTimer();
};

#endif // CDI_COMMON_CYCLECOUNTER_HPP
