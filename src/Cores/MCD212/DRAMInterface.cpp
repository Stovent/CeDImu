#include "MCD212.hpp"
#include "../../utils.hpp"

uint8_t MCD212::GetByte(const uint32_t& addr) const
{
    if(addr <= 0x4FFFFF)
        return memory[addr];
    else
        return 0;
}

uint16_t MCD212::GetWord(const uint32_t& addr)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount++;
        return memory[addr + 0x400000] << 8 | memory[addr + 0x400001];
    }
    if(addr < 0x4FFFFF)
        return memory[addr] << 8 | memory[addr + 1];
    else
        return 0;
}

uint32_t MCD212::GetLong(const uint32_t& addr)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount += 2;
        return memory[addr + 0x400000] << 24 | memory[addr + 0x400001] << 16 | memory[addr + 0x400002] << 8 | memory[addr + 0x400003];
    }
    if(addr < 0x4FFFFF)
        return memory[addr] << 24 | memory[addr + 1] << 16 | memory[addr + 2] << 8 | memory[addr + 3];
    else
        return 0;
}

void MCD212::SetByte(const uint32_t& addr, const uint8_t& data)
{
    if(addr <= 0x3FFFFF)
        memory[addr] = data;
}

void MCD212::SetWord(const uint32_t& addr, const uint16_t& data)
{
    if(addr < 0x3FFFFF)
    {
        memory[addr] = data >> 8;
        memory[addr+1] = data;
    }
}

void MCD212::SetLong(const uint32_t& addr, const uint32_t& data)
{
    if(addr < 0x3FFFFF)
    {
        memory[addr]   = data >> 24;
        memory[addr+1] = (data & 0x00FF0000) >> 16;
        memory[addr+2] = (data & 0x0000FF00) >> 8;
        memory[addr+3] = data;
    }
}
