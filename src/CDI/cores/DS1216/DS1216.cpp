#include "DS1216.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

enum DS1216Clock
{
    Hundredths,
    Seconds,
    Minutes,
    Hour,
    Day,
    Date,
    Month,
    Year,
};

static constexpr std::array<bool, 64> matchPattern = {
    0,1,0,1,1,1,0,0,
    1,0,1,0,0,0,1,1,
    0,0,1,1,1,0,1,0,
    1,1,0,0,0,1,0,1,
    0,1,0,1,1,1,0,0,
    1,0,1,0,0,0,1,1,
    0,0,1,1,1,0,1,0,
    1,1,0,0,0,1,0,1,
};

/** \brief Constructs a new timekeeper.
 * \param cdi A reference to the CDI context.
 * \param initialTime The UNIX timestamp to be used as the initial time.
 * \param state The initial state of the SRAM, or nullptr for empty SRAM. Must be 32776 (0x8000 + 8) bytes long.
 * \throw std::invalid_argument if \p state is not 32776 bytes long.
 *
 * If \p initialTime is empty, then time will continue from the time stored in the initial state at the 8 last bytes.
 * If \p initialTime is empty and no initial state is provided, the initial time used is IRTC::defaultTime.
 */
DS1216::DS1216(CDI& cdi, std::span<const uint8_t> state, std::optional<std::time_t> initialTime)
    : IRTC(cdi)
    , m_sram{}
    , m_clock{}
    , m_nsec(0.0)
    , m_internalClock(std::chrono::system_clock::from_time_t(initialTime.value_or(IRTC::defaultTime)))
    , m_patternCount(-1)
    , m_pattern(64)
{
    if(!state.empty())
    {
        if(state.size() != 32776)
            throw std::invalid_argument("DS1216 initial state size must be 32776 bytes");

        std::copy(state.begin(), state.begin() + m_sram.size(), m_sram.begin());
        std::copy(state.begin() + m_sram.size(), state.begin() + m_sram.size() + 8, m_clock.begin());
        if(initialTime.has_value())
            ClockToSRAM();
        else
            SRAMToClock();
    }
    else
    {
        m_sram.fill(0xFF);

        ClockToSRAM();
    }
}

/** \brief Destroys the timekeeper.
 * Calls Callbacks::OnSaveNVRAM.
 */
DS1216::~DS1216() noexcept
{
    ClockToSRAM();
    uint8_t nv[0x8008];
    memcpy(nv, m_sram.data(), m_sram.size());
    memcpy(&nv[0x8000], m_clock.data(), 8);

    cdi.m_callbacks.OnSaveNVRAM(nv, 0x8008);
}

/** \brief Move the internal clock to SRAM.
 */
void DS1216::ClockToSRAM()
{
    if(m_patternCount >= 0) // Do not change SRAM clock when it is being read.
        return;

    const std::chrono::hh_mm_ss hms(m_internalClock.time_since_epoch());
    const std::chrono::time_point<std::chrono::system_clock, std::chrono::days> days_duration = std::chrono::floor<std::chrono::days>(m_internalClock);
    const std::chrono::year_month_day ymd(days_duration);
    const std::chrono::weekday weekday(days_duration);

    uint8_t hour = hms.hours().count() % 24;
    if(bit<7>(m_clock[Hour])) // 12h format
    {
        int h = hour;
        if(h >= 12) // PM
        {
            hour = 0xA0;
            if(h > 12)
                h -= 12;
        }
        else // AM
        {
            hour = 0x80;
            if(h == 0)
                h = 12;
        }
        hour |= byteToPBCD(h);
    }
    else // 24h format
        hour = byteToPBCD(hour);

    static const unsigned fractional_width = std::chrono::hh_mm_ss<std::chrono::system_clock::duration>::fractional_width;
    static_assert(fractional_width >= 2, "std::chrono::hh_mm_ss must support subsecond precision");
    static const unsigned hundrethsDivider = pow(10, fractional_width - 2);

    m_clock[Hundredths] = byteToPBCD(hms.subseconds().count() / hundrethsDivider);
    m_clock[Seconds] = byteToPBCD(hms.seconds().count());
    m_clock[Minutes] = byteToPBCD(hms.minutes().count());
    m_clock[Hour]    = hour;
    m_clock[Day]     = byteToPBCD(weekday.iso_encoding()) | (m_clock[Day] & 0x30);
    m_clock[Date]    = byteToPBCD(static_cast<unsigned>(ymd.day()));
    m_clock[Month]   = byteToPBCD(static_cast<unsigned>(ymd.month()));
    m_clock[Year]    = byteToPBCD(static_cast<int>(ymd.year()) % 100);
}

/** \brief Move the SRAM clock into the internal clock.
 */
void DS1216::SRAMToClock()
{
    uint8_t hour;
    if(bit<7>(m_clock[Hour])) // 12h format
    {
        hour = PBCDToByte(bits<0, 4>(m_clock[Hour]));
        if(hour == 12)
            hour = 0;
        if(bit<5>(m_clock[Hour])) // PM
            hour += 12;
    }
    else
        hour = PBCDToByte(bits<0, 5>(m_clock[Hour]));

    m_nsec = PBCDToByte(m_clock[Hundredths]) * 10'000'000.0;
    const std::chrono::seconds seconds = std::chrono::seconds(PBCDToByte(bits<0, 6>(m_clock[Seconds])));
    const std::chrono::minutes minutes = std::chrono::minutes(PBCDToByte(bits<0, 6>(m_clock[Minutes])));
    const std::chrono::hours hours = std::chrono::hours(hour);
    const std::chrono::hh_mm_ss hms(seconds + minutes + hours);

    const std::chrono::day day(PBCDToByte(bits<0, 5>(m_clock[Date])));
    const std::chrono::month month(PBCDToByte(bits<0, 4>(m_clock[Month])));
    unsigned y = 1900 + PBCDToByte(m_clock[Year]); // Only the two last digit of the year are stored.
    if(y < 1970) // Treat 1970 and above as 1970 up to 1999, and below 1970 as being 2000's up to 2069.
        y += 100;
    const std::chrono::year year(y);
    const std::chrono::year_month_day ymd(year, month, day);

    m_internalClock = static_cast<std::chrono::sys_days>(ymd);
    m_internalClock += hms.to_duration();
}

void DS1216::PushPattern(const bool bit)
{
    m_pattern.push_front(bit);
    m_pattern.pop_back();
    if(std::equal(m_pattern.begin(), m_pattern.end(), matchPattern.begin(), matchPattern.end()))
    {
        ClockToSRAM();
        m_patternCount = 0;
    }
}

void DS1216::IncrementClockAccess()
{
    if(++m_patternCount >= 64)
        m_patternCount = -1;
}

void DS1216::IncrementClock(const double ns)
{
    if(bit<5>(m_clock[Day])) // OSC bit
        return;

    m_nsec += ns;
    while(m_nsec >= 10'000'000.0)
    {
        m_nsec -= 10'000'000.0;
        m_internalClock += std::chrono::milliseconds(10);
    }
}

uint8_t DS1216::PeekByte(const uint16_t addr) const noexcept
{
    if(addr < m_sram.size())
        return m_sram[addr];

    return m_clock.at(addr - m_sram.size());
}

uint8_t DS1216::GetByte(const uint16_t addr, const BusFlags flags)
{
    if(m_patternCount < 0)
    {
        LOG(if(flags.log && cdi.m_callbacks.HasOnLogMemoryAccess()) \
                cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::RTC, "Get", "SRAM", cdi.m_cpu.currentPC, addr, m_sram[addr]});)
        return m_sram[addr];
    }

    const uint8_t reg = m_patternCount / 8;
    const uint8_t shift = m_patternCount % 8;
    const bool bit = m_clock[reg] & (1 << shift);

    LOG(if(flags.log && cdi.m_callbacks.HasOnLogMemoryAccess()) \
            cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::RTC, "Get", "clock", cdi.m_cpu.currentPC, addr, bit});)

    IncrementClockAccess();
    return bit;
}

void DS1216::SetByte(const uint16_t addr, const uint8_t data, const BusFlags flags)
{
    const bool lsb = bit<0>(data);

    if(m_patternCount < 0)
    {
        LOG(if(flags.log && cdi.m_callbacks.HasOnLogMemoryAccess()) \
                cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::RTC, "Set", "SRAM", cdi.m_cpu.currentPC, addr, data});)
        m_sram[addr] = data;
        PushPattern(lsb);
    }
    else
    {
        const uint8_t reg = m_patternCount / 8;
        const uint8_t shift = m_patternCount % 8;

        LOG(if(flags.log && cdi.m_callbacks.HasOnLogMemoryAccess()) \
                cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::RTC, "Set", "clock", cdi.m_cpu.currentPC, addr, lsb});)

        m_clock[reg] &= ~(1 << shift);
        m_clock[reg] |= lsb << shift;

        IncrementClockAccess();
        if(m_patternCount == -1)
            SRAMToClock();
    }
}
