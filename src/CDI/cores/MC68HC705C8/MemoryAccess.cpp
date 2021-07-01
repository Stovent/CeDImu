#include "MC68HC705C8.hpp"
#include "../../common/Callbacks.hpp"

uint8_t MC68HC705C8::GetByte(const uint16_t addr)
{
    LOG(if(addr == 0x0011) { fprintf(instructions, "\tGet SCI #0x%X\n", memory[addr]); })
    if(addr < SLAVE_MEMORY_SIZE)
        return memory[addr];

    LOG(fprintf(instructions, "%X\tGet byte OUT OF RANGE at 0x%X\n", currentPC, addr))
    return 0;
}

void MC68HC705C8::SetByte(const uint16_t addr, const uint8_t value)
{
    LOG(if(addr == 0x0011) { fprintf(instructions, "\tSet SCI #0x%X\n", value); })
    if(addr < SLAVE_MEMORY_SIZE)
    {
        memory[addr] = value;
        return;
    }

    LOG(fprintf(instructions, "%X\tSet byte OUT OF RANGE at 0x%X : %d %d 0x%X\n", currentPC, addr, (int8_t)value, value, value);)
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
