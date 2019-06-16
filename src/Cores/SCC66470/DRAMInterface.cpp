#include "SCC66470.hpp"

uint8_t SCC66470::GetByte(const uint32_t& addr)
{
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
        return memory[addr];
    else
        return 0;
}

uint16_t SCC66470::GetWord(const uint32_t& addr)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount += 1;
        return memory[addr+0x180000] << 8 | memory[addr+0x180001];

    }
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
        return memory[addr] << 8 | memory[addr+1];
    else
        return 0;
}

uint32_t SCC66470::GetLong(const uint32_t& addr)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount += 2;
        return memory[addr+0x180000] << 24 | memory[addr+0x180001] << 16 | memory[addr+0x180002] << 8 | memory[addr+0x180003];

    }
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
        return memory[addr] << 24 | memory[addr+1] << 16 | memory[addr+2] << 8 | memory[addr+3];
    else
        return 0;
}

void SCC66470::SetByte(const uint32_t& addr, const uint8_t& data)
{
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
        memory[addr] = data;
}

void SCC66470::SetWord(const uint32_t& addr, const uint16_t& data)
{
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
    {
        memory[addr] = (data & 0xFF00) >> 8;
        memory[addr + 1] = data & 0x00FF;
    }
}

void SCC66470::SetLong(const uint32_t& addr, const uint32_t& data)
{
    if((addr < 0x100000 || addr > 0x17FFFF) && addr < 0x200000)
    {
        memory[addr]     = (data & 0xFF000000) >> 24;
        memory[addr + 1] = (data & 0x00FF0000) >> 16;
        memory[addr + 2] = (data & 0x0000FF00) >> 8;
        memory[addr + 3] = (data & 0x000000FF);
    }
}
