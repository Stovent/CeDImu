#include "M48T08.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

/** \brief Constructs a new M48T08 timekeeper.
 * \param cdi A reference to the CDI context.
 * \param initialTime The UNIX timestamp to be used as the initial time.
 * \param state The initial state of the SRAM, or nullptr for empty SRAM. Must be 8192 bytes long.
 * \throw std::invalid_argument if \p state is not 8192 bytes long.
 *
 * If \p initialTime is empty, then time will continue from the time stored in the initial state at the 8 last bytes.
 * If \p initialTime is empty and no initial state is provided, the initial time used is IRTC::defaultTime.
 */
M48T08::M48T08(CDI& cdi, std::span<const uint8_t> state, std::optional<std::time_t> initialTime)
    : IRTC(cdi)
    , m_sram{}
    , m_nsec(0.0)
    , m_internalClock(std::chrono::system_clock::from_time_t(initialTime.value_or(IRTC::defaultTime)))
{
    if(!state.empty())
    {
        if(state.size() != m_sram.size())
            throw std::invalid_argument("M48T08 initial state size must be 8192 bytes");

        std::copy(state.begin(), state.begin() + m_sram.size(), m_sram.begin());
        if(initialTime.has_value())
            ClockToSRAM();
        else
            SRAMToClock();
    }
    else
    {
        std::fill(m_sram.begin(), m_sram.begin() + 0x1FF8, 0xFF);
        std::fill(m_sram.begin() + 0x1FF8, m_sram.end(), 0);

        ClockToSRAM();
    }
    m_sram[Control] = 0;
}

/** \brief Destroys the timekeeper.
 *
 * Calls Callbacks::OnSaveNVRAM.
 */
M48T08::~M48T08() noexcept
{
    ClockToSRAM();
    cdi.m_callbacks.OnSaveNVRAM(m_sram.data(), m_sram.size());
}

/** \brief Moves the internal clock to SRAM.
 */
void M48T08::ClockToSRAM()
{
    if(m_sram[Control] & 0xC0)
        return;

    const std::chrono::hh_mm_ss hms(m_internalClock.time_since_epoch());
    const std::chrono::time_point<std::chrono::system_clock, std::chrono::days> days_duration = std::chrono::floor<std::chrono::days>(m_internalClock);
    const std::chrono::year_month_day ymd(days_duration);
    const std::chrono::weekday weekday(days_duration);

    m_sram[Seconds] = byteToPBCD(hms.seconds().count()) | (m_sram[Seconds] & 0x80);
    m_sram[Minutes] = byteToPBCD(hms.minutes().count());
    m_sram[Hours]   = byteToPBCD(hms.hours().count() % 24);
    m_sram[Day]     = weekday.iso_encoding() | (m_sram[Day] & 0x40); // TODO: verify that 7 = sunday.
    m_sram[Date]    = byteToPBCD(static_cast<unsigned>(ymd.day()));
    m_sram[Month]   = byteToPBCD(static_cast<unsigned>(ymd.month()));
    m_sram[Year]    = byteToPBCD(static_cast<int>(ymd.year()) % 100);
}

/** \brief Moves the SRAM clock into the internal clock.
 */
void M48T08::SRAMToClock()
{
    const std::chrono::seconds seconds = std::chrono::seconds(PBCDToByte(bits<0, 6>(m_sram[Seconds])));
    const std::chrono::minutes minutes = std::chrono::minutes(PBCDToByte(bits<0, 6>(m_sram[Minutes])));
    const std::chrono::hours hours = std::chrono::hours(PBCDToByte(bits<0, 5>(m_sram[Hours])));
    const std::chrono::hh_mm_ss hms(seconds + minutes + hours);

    const std::chrono::day day(PBCDToByte(bits<0, 5>(m_sram[Date])));
    const std::chrono::month month(PBCDToByte(bits<0, 4>(m_sram[Month])));
    unsigned y = 1900 + PBCDToByte(m_sram[Year]); // Only the two last digit of the year are stored.
    if(y < 1970) // Treat 1970 and above as 1970 up to 1999, and below 1970 as being 2000's up to 2069.
        y += 100;
    const std::chrono::year year(y);
    const std::chrono::year_month_day ymd(year, month, day);

    m_internalClock = static_cast<std::chrono::sys_days>(ymd);
    m_internalClock += hms.to_duration();
}

/** \brief Increments the internal clock.
 *
 * \param ns The number of nanoseconds to increment the clock by.
 *
 * Increment only occurs if the STOP bit is not set.
 */
void M48T08::IncrementClock(const double ns)
{
    if(bit<7>(m_sram[Seconds])) // STOP bit
        return;

    m_nsec += ns;
    while(m_nsec >= 1'000'000'000.0)
    {
        m_nsec -= 1'000'000'000.0;
        m_internalClock += std::chrono::seconds(1);
        ClockToSRAM();
    }
}

uint8_t M48T08::PeekByte(const uint16_t addr) const noexcept
{
    return m_sram.at(addr);
}

/** \brief Returns the byte at the given address in SRAM.
 *
 * \param addr The address of the byte.
 * \return The byte at the given address.
 *
 * In order to read the clock, the READ bit must be set in the control register using SetByte(0x1FF8).
 */
uint8_t M48T08::GetByte(const uint16_t addr, const BusFlags flags)
{
    LOG(if(flags.log && cdi.m_callbacks.HasOnLogMemoryAccess()) \
            cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::RTC, "Get", "Byte", cdi.m_cpu.currentPC, addr, m_sram[addr]});)

    return m_sram[addr];
}

/** \brief Sets a byte at the given address in SRAM.
 *
 * \param addr The address of the byte.
 * \param data The value of the byte to set.
 *
 * If the address is the control register and the WRITE bit is going from set to unset, the clock in SRAM is moved in the internal clock.
 * If the address is the control register and the READ  bit is going from unset to set, the internal clock is moved in SRAM to be read.
 */
void M48T08::SetByte(const uint16_t addr, const uint8_t data, const BusFlags flags)
{
    LOG(if(flags.log && cdi.m_callbacks.HasOnLogMemoryAccess()) \
            cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::RTC, "Set", "Byte", cdi.m_cpu.currentPC, addr, data});)

    if(addr == Control)
    {
        if(bit<6>(data) && !bit<6>(m_sram[Control]))
            ClockToSRAM();
        if(!bit<7>(data) && bit<7>(m_sram[Control]))
            SRAMToClock();
    }

    m_sram[addr] = data;
}
