#include "DS1216.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>

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

constexpr std::array<bool, 64> matchPattern =
{0,1,0,1,1,1,0,0, 1,0,1,0,0,0,1,1, 0,0,1,1,1,0,1,0, 1,1,0,0,0,1,0,1, 0,1,0,1,1,1,0,0, 1,0,1,0,0,0,1,1, 0,0,1,1,1,0,1,0, 1,1,0,0,0,1,0,1};

DS1216::DS1216(CDI& idc) :
    IRTC(idc),
    internalClock{599616000, 0.0},
    clock{0},
    sram{0xFF},
    patternCount(-1),
    pattern(64)
{
}

/** \brief Move the internal clock to SRAM.
 */
void DS1216::ClockToSRAM()
{
    if(patternCount >= 0) // Do not change SRAM clock when it is being read.
        return;

    const std::tm* gmt = std::gmtime(&internalClock.sec);
    uint8_t hour = byteToPBCD(gmt->tm_hour);
    if(clock[Hour] & 0x80) // 12h format
    {
        int h = gmt->tm_hour;
        if(h >= 12) // PM
        {
            hour = 0xA0;
            if(h > 12)
                h -= 12;
        }
        else // AM
        {
            hour = 0x80;
            h = h == 0 ? 12 : h;
        }
        hour |= byteToPBCD(h);
    }

    clock[Hundredths] = (int)(internalClock.nsec / 10'000'000.0);
    clock[Seconds] = byteToPBCD(gmt->tm_sec);
    clock[Minutes] = byteToPBCD(gmt->tm_min);
    clock[Hour]    = hour;
    clock[Day]     = byteToPBCD(gmt->tm_wday ? gmt->tm_wday : 7) | (clock[Day] & 0x30);
    clock[Date]    = byteToPBCD(gmt->tm_mday);
    clock[Month]   = byteToPBCD(gmt->tm_mon + 1);
    clock[Year]    = byteToPBCD(gmt->tm_year);
}

/** \brief Move the SRAM clock into the internal clock.
 */
void DS1216::SRAMToClock()
{
    int hour;
    if(clock[Hour] & 0x80) // 12h format
    {
        hour = PBCDToByte(clock[Hour] & 0x1F);
        if(clock[Hour] & 0x20) // PM
        {
            hour = 12 + (hour == 12 ? 0 : hour);
        }
        else // AM
        {
            hour = (hour == 12 ? 0 : hour);
        }
    }
    else
        hour = PBCDToByte(clock[Hour] & 0x3F);

    std::tm gmt;
    gmt.tm_sec  = PBCDToByte(clock[Seconds]);
    gmt.tm_min  = PBCDToByte(clock[Minutes]);
    gmt.tm_hour = hour;
    gmt.tm_mday = PBCDToByte(clock[Date]);
    gmt.tm_mon  = PBCDToByte(clock[Month]) - 1;
    gmt.tm_year = PBCDToByte(clock[Year]); if(gmt.tm_year < 70) gmt.tm_year += 100;
    gmt.tm_isdst = 0;
    internalClock.sec = std::mktime(&gmt);
    internalClock.nsec = PBCDToByte(clock[Hundredths]) * 10'000'000.0;
}

void DS1216::PushPattern(const bool bit)
{
    pattern.push_front(bit);
    pattern.pop_back();
    if(std::equal(pattern.begin(), pattern.end(), matchPattern.begin(), matchPattern.end()))
    {
        ClockToSRAM();
        patternCount = 0;
    }
}

void DS1216::IncrementClockAccess()
{
    if(++patternCount >= 64)
    {
        patternCount = -1;
    }
}

void DS1216::IncrementClock(const double ns)
{
    if(clock[Day] & 0x20) // OSC bit
        return;

    internalClock.nsec += ns;
    while(internalClock.nsec >= 1'000'000'000.0)
    {
        internalClock.sec++;
        internalClock.nsec -= 1'000'000'000.0;
    }
}

uint8_t DS1216::GetByte(const uint16_t addr)
{
    if(patternCount < 0)
    {
        LOG(if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({"RTC", "Get", "SRAM", cdi.board->cpu.currentPC, addr, sram[addr]});)
        return sram[addr];
    }

    const uint8_t reg = patternCount / 8;
    const uint8_t shift = patternCount % 8;
    const bool bit = clock[reg] & (1 << shift);

    LOG(if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({"RTC", "Get", "clock", cdi.board->cpu.currentPC, addr, bit});)

    IncrementClockAccess();
    return bit;
}

void DS1216::SetByte(const uint16_t addr, const uint8_t data)
{
    if(patternCount < 0)
    {
        LOG(if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({"RTC", "Set", "SRAM", cdi.board->cpu.currentPC, addr, data});)
        sram[addr] = data;
        PushPattern(data & 1);
    }
    else
    {
        const uint8_t reg = patternCount / 8;
        const uint8_t shift = patternCount % 8;
        const bool bit = data & 1;

        LOG(if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({"RTC", "Set", "clock", cdi.board->cpu.currentPC, addr, bit});)

        clock[reg] &= ~(1 << shift);
        clock[reg] |= bit << shift;

        IncrementClockAccess();
        if(patternCount == -1)
            SRAMToClock();
    }
}
