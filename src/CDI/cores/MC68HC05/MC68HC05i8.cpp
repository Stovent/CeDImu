#include "MC68HC05i8.hpp"

#include <cstring>

#define SLAVE_FREQUENCY (4'000'000)

MC68HC05i8::MC68HC05i8(const void* internalMemory, uint16_t size)
    : MC68HC05(memory.size())
    , memory{0}
    , pendingCycles{0}
{
    if(size > memory.size())
        size = memory.size();

    if(internalMemory != nullptr)
        memcpy(memory.data(), internalMemory, size);

    Reset();
}

void MC68HC05i8::Reset()
{
    A = 0;
    X = 0;
    PC = ((uint16_t)GetMemory(0x3FFE) << 8) | GetMemory(0x3FFF);
    SP = 0xFF;
    CCR |= 0xE8;
    waitStop = false;

    pendingCycles = 0;
    irqPin = true;
}

void MC68HC05i8::IncrementTime(double ns)
{
    if(!waitStop)
    {
        pendingCycles += ns / SLAVE_FREQUENCY;
        while(pendingCycles > 0)
        {
            pendingCycles -= Interpreter();
        }
    }
}

uint8_t MC68HC05i8::GetMemory(const uint16_t addr)
{
    if(addr < memory.size())
        return memory[addr];

    // TODO: 4.1.3 Illegal Address Reset
    printf("[MC68HC05i8] Read at %X out of range\n", addr);
    return 0;
}

void MC68HC05i8::SetMemory(const uint16_t addr, const uint8_t value)
{
    if(addr < 0x0130)
    {
        memory[addr] = value;
        return;
    }

    // TODO: 4.1.3 Illegal Address Reset
    printf("[MC68HC05i8] Write at %X (%d) out of range\n", addr, value);
}

void MC68HC05i8::Stop()
{

}

void MC68HC05i8::Wait()
{

}
