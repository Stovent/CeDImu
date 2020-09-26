#include "MC68HC705C8.hpp"

uint8_t MC68HC705C8::GetByte(const uint16_t addr)
{
    if(addr < 0x2000)
        return memory[addr];

    return 0;
}

void MC68HC705C8::SetByte(const uint16_t addr, const uint8_t value)
{
    if(addr < 0x2000)
        memory[addr] = value;
}

uint8_t MC68HC705C8::GetNextByte()
{
    const uint8_t byte = GetByte(PC);
    PC++;
    return byte;
}

void MC68HC705C8::PushByte(const uint8_t data)
{
    memory[SP--] = data;
    if(SP < 0xC0)
        SP = 0xFF;
}

uint8_t MC68HC705C8::PopByte()
{
    if(SP == 0xFF)
        SP = 0xBF;
    return memory[++SP];
}
