#include "MCD212.hpp"

uint8_t MCD212::GetByte(const uint32_t& addr) const
{
    if(addr < 0x7FFFF)
        return (memory[addr] & 0xFF00) >> 8;
    else
        return 0;
}

uint16_t MCD212::GetWord(const uint32_t& addr)
{
    if(addr < 0x7FFFF)
        return memory[addr];
    else
        return 0;
}

uint32_t MCD212::GetLong(const uint32_t& addr)
{
    if(addr < 0x7FFFF)
        return (memory[addr] << 8) | memory[addr+1];
    else
        return 0;
}

void MCD212::SetByte(const uint32_t& addr, const uint8_t& data)
{

}

void MCD212::SetWord(const uint32_t& addr, const uint16_t& data)
{

}

void MCD212::SetLong(const uint32_t& addr, const uint32_t& data)
{

}
