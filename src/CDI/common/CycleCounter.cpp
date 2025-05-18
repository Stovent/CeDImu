#include "CycleCounter.hpp"

#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(ret, test) if(!(test)) return ret

constexpr int testCycleCounter()
{
    using Reference = std::chrono::duration<CycleRep, std::ratio<1, 10>>;

    constexpr CycleCounter<3, Reference> cycles3{CycleRep{1}};
    static_assert(cycles3.m_cycles.count() == 1);
    // static_assert(cycles3.m_subCycle.count() == 0);

    static_assert(std::chrono::duration_cast<std::chrono::milliseconds>(Reference{19}).count() == 1900);
    static_assert(cycles3.IncrementTo(Reference{13}).m_cycles.count() == 3);
    static_assert(cycles3.IncrementTo(Reference{14}).m_cycles.count() == 4);
    static_assert(cycles3.IncrementTo(Reference{19}).m_cycles.count() == 5);
    static_assert(cycles3.IncrementTo(Reference{20}).m_cycles.count() == 6);

    // constexpr CycleCounter<3, Reference> cycles2 = CycleCounter<3, Reference>{} + CycleCounter<10, Reference>{10};
    // static_assert(cycles2.m_cycles.count() == 3); // 3
    // static_assert(cycles2.m_subCycle.count() == 0);

    // static_assert(std::chrono::duration_cast<GPUCycle>(cpu).count() == GPU_FREQUENCY);
    // static_assert(std::chrono::duration_cast<TickCycle>(cpu).count() == TICK_FREQUENCY);

    return 0;
}
static_assert(testCycleCounter() == 0);

constexpr int testCycleTimer()
{
    using Reference = std::chrono::duration<CycleRep, std::ratio<1, 6>>;

    CycleTimer<3> cycleTimer{2};
    ASSERT(10, cycleTimer.m_lastEvent.count() == 0);
    ASSERT(20, cycleTimer.m_delay.count() == 2);

    ASSERT(30, cycleTimer.IncrementTo(Reference{1}) == false);
    ASSERT(31, cycleTimer.m_lastEvent.count() == 0);
    ASSERT(40, cycleTimer.IncrementTo(Reference{2}) == false);
    ASSERT(50, cycleTimer.IncrementTo(Reference{3}) == false);
    ASSERT(60, cycleTimer.IncrementTo(Reference{4}) == true);
    ASSERT(61, cycleTimer.m_lastEvent.count() == 2);
    ASSERT(70, cycleTimer.IncrementTo(Reference{5}) == false);

    return 0;
}
static_assert(testCycleTimer() == 0);
