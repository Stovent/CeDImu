#include "MC68HC705C8.hpp"

#include <fstream>

MC68HC705C8::MC68HC705C8(const void* bios, uint16_t size)
{
    memory = new uint8_t[0x2000];
    currentOpcode = 0;
    currentPC = 0;

    if(size > 0x2000)
        size = 0x2000;
    memcpy(memory, bios, size);
}

MC68HC705C8::~MC68HC705C8()
{
    delete[] memory;
}

void MC68HC705C8::Reset()
{
    A = 0;
    X = 0;
    PC = (GetByte(0x1FFE) << 8) | GetByte(0x1FFF);
    SP.byte = 0xFF;
    CCR.byte = 0xE0;
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
