#include "DS1216.hpp"

DS1216::DS1216(CDI& idc) :
    IRTC(idc),
    clock{0},
    sram{0}
{
}

void DS1216::IncrementClock(const double ns)
{
}

uint8_t DS1216::GetByte(const uint16_t addr) const
{
    return sram[addr];
}

void DS1216::SetByte(const uint16_t addr, const uint8_t data)
{
    sram[addr] = data;
}
