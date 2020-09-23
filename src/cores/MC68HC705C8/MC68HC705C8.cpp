#include "MC68HC705C8.hpp"
#include "../../utils.hpp"

#include <cstring>

MC68HC705C8::MC68HC705C8(const void* bios, uint16_t size)
{
    memory = new uint8_t[0x2000];
    currentOpcode = 0;
    currentPC = 0;
    OPEN_LOG(instructions, "slave_instructions.txt")

    if(size > 0x2000)
        size = 0x2000;
    memcpy(memory, bios, size);
    Reset();
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
    SP = 0xFF;
    CCR |= 0xE8;

    pendingCycles = 0;
}
