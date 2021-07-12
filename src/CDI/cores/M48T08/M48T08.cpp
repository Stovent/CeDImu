#include "M48T08.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../common/utils.hpp"

#include <cstring>
#include <fstream>

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

/** \brief Construct a new M48T08 timekeeper.
 * \param idc A reference to the CDI context.
 * \param initialTime The timestamp to be used as the initial time.
 * \param state The initial state of the SRAM, or nullptr for empty SRAM. Must be 8192 bytes long.
 *
 * If \p initialTime is 0, then time will continue from the time stored in the initial state at the 8 last bytes.
 */
M48T08::M48T08(CDI& idc, std::time_t initialTime, const uint8_t* state) :
    IRTC(idc),
    internalClock{initialTime, 0.0}
{
    if(state)
    {
        std::copy(state, &state[sram.size()], sram.begin());
        if(initialTime)
            ClockToSRAM();
        else
            SRAMToClock();
    }
    else
    {
        sram.fill(0xFF);
        std::fill(&sram[0x1FF8], sram.end(), 0);

        if(initialTime == 0)
            initialTime = IRTC::defaultTime;

        internalClock.sec = initialTime;
        ClockToSRAM();
    }
    sram[Control] = 0;
}

/** \brief Destroy the timekeeper.
 *
 * Calls the \p OnSaveNVRAM callback.
 */
M48T08::~M48T08()
{
    ClockToSRAM();
    cdi.callbacks.OnSaveNVRAM(sram.data(), sram.size());
}

/** \brief Move the internal clock to SRAM.
 */
void M48T08::ClockToSRAM()
{
    if(sram[Control] & 0xC0)
        return;

    const std::tm* gmt = std::gmtime(&internalClock.sec);
    sram[Seconds] = byteToPBCD(gmt->tm_sec) | (sram[Seconds] & 0x80);
    sram[Minutes] = byteToPBCD(gmt->tm_min);
    sram[Hours]   = byteToPBCD(gmt->tm_hour);
    sram[Day]     = byteToPBCD(gmt->tm_wday ? gmt->tm_wday : 7) | (sram[Day] & 0x40); // TODO: verify that 1 = monday, 0 = sunday, etc.
    sram[Date]    = byteToPBCD(gmt->tm_mday);
    sram[Month]   = byteToPBCD(gmt->tm_mon + 1);
    sram[Year]    = byteToPBCD(gmt->tm_year);
}

/** \brief Move the SRAM clock into the internal clock.
 */
void M48T08::SRAMToClock()
{
    std::tm gmt;
    gmt.tm_sec  = PBCDToByte(sram[Seconds] & 0x7F);
    gmt.tm_min  = PBCDToByte(sram[Minutes]);
    gmt.tm_hour = PBCDToByte(sram[Hours]);
    gmt.tm_mday = PBCDToByte(sram[Date]);
    gmt.tm_mon  = PBCDToByte(sram[Month]) - 1;
    gmt.tm_year = PBCDToByte(sram[Year]); gmt.tm_year += (gmt.tm_year >= 70 ? 0 : 100);
    gmt.tm_isdst = 0;
    internalClock.sec = std::mktime(&gmt);
    internalClock.nsec = 0.0;
}

/** \brief Increment the internal clock.
 *
 * \param ns The number of nanoseconds to increment the clock by.
 *
 * Increment only occurs if the STOP bit is not set.
 */
void M48T08::IncrementClock(const double ns)
{
    if(sram[Seconds] & 0x80) // STOP bit
        return;

    internalClock.nsec += ns;
    while(internalClock.nsec >= 1'000'000'000.0)
    {
        internalClock.sec++;
        internalClock.nsec -= 1'000'000'000.0;
        ClockToSRAM();
    }
}

/** \brief Get a byte in SRAM.
 *
 * \param addr The address of the byte.
 * \return The byte at the given address.
 *
 * In order to read the clock, the READ bit must be set in the control register using SetByte(0x1FF8).
 */
uint8_t M48T08::GetByte(const uint16_t addr)
{
    LOG(if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({"RTC", "Get", "Byte", cdi.board->cpu.currentPC, addr, sram[addr]});)
    return sram[addr];
}

/** \brief Set a byte in SRAM.
 *
 * \param addr The address of the byte.
 * \param data The value of the byte to set.
 *
 * If the address is the control register and the WRITE bit is going from set to unset, the clock in SRAM is moved in the internal clock.
 * If the address is the control register and the READ  bit is going from unset to set, the internal clock is moved in SRAM to be read.
 */
void M48T08::SetByte(const uint16_t addr, const uint8_t data)
{
    LOG(if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({"RTC", "Set", "Byte", cdi.board->cpu.currentPC, addr, data});)
    if(addr == Control)
    {
        if(data & 0x40 && !(sram[Control] & 0x40))
            ClockToSRAM();
        if(!(data & 0x80) && sram[Control] & 0x80)
            SRAMToClock();
    }

    sram[addr] = data;
}
