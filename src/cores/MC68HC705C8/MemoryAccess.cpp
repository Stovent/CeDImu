#include "MC68HC705C8.hpp"
#include "../../utils.hpp"

uint8_t MC68HC705C8::GetByte(const uint16_t addr)
{
    if(addr < SLAVE_MEMORY_SIZE)
        return memory[addr];

    LOG(instructions << "\tGet byte OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

void MC68HC705C8::SetByte(const uint16_t addr, const uint8_t value)
{
    if(addr < SLAVE_MEMORY_SIZE)
    {
        memory[addr] = value;
        return;
    }

    LOG(instructions << "\tSet byte OUT OF RANGE at 0x" << addr << " : 0x" << (uint16_t)value << " (" << std::dec << (int16_t)value << ")" << std::endl)
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
