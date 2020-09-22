#include "MC68HC705C8.hpp"

uint8_t GetByteIndexed()
{

}

void SetByteIndexed(const uint8_t value)
{

}

uint8_t GetByteIndexed8()
{

}

void SetByteIndexed8(const uint8_t value)
{

}

uint8_t GetByteIndexed16()
{

}

void SetByteIndexed16(const uint8_t value)
{

}

uint8_t GetByteRelative()
{

}

void SetByteRelative(const uint8_t value)
{

}

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
