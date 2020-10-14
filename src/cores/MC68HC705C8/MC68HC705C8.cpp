#include "MC68HC705C8.hpp"
#include "../../utils.hpp"

#include <cstring>

MC68HC705C8::MC68HC705C8(const void* bios, uint16_t size)
{
    memory = new uint8_t[SLAVE_MEMORY_SIZE];
    currentOpcode = 0;
    currentPC = 0;
    OPEN_LOG(instructions, "slave_instructions.txt")

    if(size > SLAVE_MEMORY_SIZE)
        size = SLAVE_MEMORY_SIZE;
    if(bios != nullptr)
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
    SP = 0xFF;
    CCR |= 0xE8;
    waitStop = false;

    pendingCycles = 0;
    irqPin = true;
}

void MC68HC705C8::IRQ()
{
    irqPin = false;
}
