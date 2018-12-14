#include "SCC66470.hpp"

int8_t SCC66470::GetByte(const uint32_t& addr) const
{
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
        return memory[addr];
    else
        return 0;
}

int16_t SCC66470::GetWord(const uint32_t& addr) const
{
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
        return memory[addr+1] << 8 | memory[addr];
    else
        return 0;
}

int32_t SCC66470::GetLong(const uint32_t& addr) const
{
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
        return memory[addr+3] << 24 | memory[addr+2] << 16 | memory[addr+1] << 8 | memory[addr];
    else
        return 0;
}

void SCC66470::SetByte(const uint32_t& addr, const int8_t& data)
{
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
        memory[addr] = data;
}

void SCC66470::SetWord(const uint32_t& addr, const int16_t& data)
{
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
    {
        memory[addr] = (data & 0xFF00) >> 8;
        memory[addr + 1] = data & 0x00FF;
    }
}

void SCC66470::SetLong(const uint32_t& addr, const int32_t& data)
{
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
    {
        memory[addr]     = (data & 0xFF000000) >> 24;
        memory[addr + 1] = (data & 0x00FF0000) >> 16;
        memory[addr + 2] = (data & 0x0000FF00) >> 8;
        memory[addr + 3] = (data & 0x000000FF);
    }
}
