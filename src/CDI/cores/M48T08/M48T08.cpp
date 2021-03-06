#include "M48T08.hpp"
#include "../../common/utils.hpp"

#include <cstring>
#include <fstream>

/** \brief Construct a new timekeeper.
 *
 * \param useCurrentTime If true, clock is set to the current UTC date. If false, the last date is used.
 *
 * If existing, loads its initial state from a file named "sram.bin".
*/
M48T08::M48T08(const bool useCurrentTime)
{
    std::ifstream in("sram.bin", std::ios::in | std::ios::binary);
    if(in)
    {
        in.read((char*)sram, 8192);
        in.close();
    }
    else
        memset(sram, 0xFF, 8192);
    sram[Control] = 0;

    internalClock.nsec = 0;
    if(useCurrentTime)
    {
        std::time(&internalClock.sec);
        ClockToSRAM();
    }
    else
        SRAMToClock();
}

/** \brief Destroy the timekeeper.
 *
 * It writes its content in a file named "sram.bin".
*/
M48T08::~M48T08()
{
    ClockToSRAM();
    std::ofstream out("sram.bin", std::ios::out | std::ios::binary);
    out.write((char*)sram, 8192);
    out.close();
}

/** \brief Increment the internal clock.
 *
 * \param ns The number of nanoseconds to increment the clock by.
 *
 * Increment only occurs if the READ or WRITE bit are not set.
*/
void M48T08::IncrementClock(const size_t ns)
{
    if(sram[Control] & 0xC0)
        return;

    internalClock.nsec += ns;
    while(internalClock.nsec >= 1'000'000'000)
    {
        internalClock.sec++;
        internalClock.nsec -= 1'000'000'000;
    }
}

/** \brief Move the internal clock to SRAM.
*/
void M48T08::ClockToSRAM()
{
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
    gmt.tm_wday = PBCDToByte((sram[Day] & 0x7) == 7 ? 0 : (sram[Day] & 0x7));
    gmt.tm_mday = PBCDToByte(sram[Date]);
    gmt.tm_mon  = PBCDToByte(sram[Month] - 1);
    gmt.tm_year = PBCDToByte(sram[Year]); gmt.tm_year += (gmt.tm_year >= 70 ? 0 : 100);
    gmt.tm_isdst = 0;
    internalClock.sec = std::mktime(&gmt);
    internalClock.nsec = 0;
}

/** \brief Get a byte in SRAM.
 *
 * \param addr The address of the byte.
 * \return The byte at the given address.
 *
 * In order to read the clock, the READ bit must be set in the control register using SetByte(0x1FF8).
*/
uint8_t M48T08::GetByte(const uint16_t addr) const
{
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
    if(addr == Control)
    {
        if(data & 0x40 && !(sram[Control] & 0x40))
            ClockToSRAM();
        if(!(data & 0x80) && sram[Control] & 0x80)
            SRAMToClock();
    }

    sram[addr] = data;
}
