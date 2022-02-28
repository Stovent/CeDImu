#include "MC68HC05i8.hpp"

#include <cstring>

#define SLAVE_FREQUENCY (4'000'000)

MC68HC05i8::MC68HC05i8(const void* internalMemory, uint16_t size)
    : MC68HC05(memory.size())
    , memory{0}
    , pendingCycles(0)
{
    if(internalMemory != nullptr)
    {
        if(size > memory.size())
            size = memory.size();
        memcpy(memory.data(), internalMemory, size);
    }

    Reset();
    // TODO:
    // 3.1.5.2 Interrupt latch
    // 4.1.3 Illegal Address Reset
}

void MC68HC05i8::Reset()
{
    MC68HC05::Reset();

    pendingCycles = 0;
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
        return memory[addr]; // TODO: GetIO(addr) ?

    printf("[MC68HC05i8] Read at 0x%X out of range\n", addr);
    return 0;
}

void MC68HC05i8::SetMemory(const uint16_t addr, const uint8_t value)
{
    if(addr < 0x0040)
        return SetIO(addr, value);

    if(addr >= 0x0050 && addr < 0x0130)
    {
        memory[addr] = value;
        return;
    }

    printf("[MC68HC05i8] Write at 0x%X (%d) out of range\n", addr, value);
}

void MC68HC05i8::SetIO(uint16_t addr, uint8_t value)
{

}

void MC68HC05i8::Stop()
{

}

void MC68HC05i8::Wait()
{

}
