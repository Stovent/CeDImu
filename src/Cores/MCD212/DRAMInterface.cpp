#include "../../utils.hpp"
#include "MCD212.hpp"

uint8_t MCD212::GetByte(const uint32_t& addr)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        out << std::hex << app->cpu->PC << "\tGet register: " << addr << std::endl;
        return registers[addr-0x4FFFE0] & 0x00FF;
    }
    else if(addr <= 0x4FFFDF)
    {
        out << std::hex << app->cpu->PC << "\tGet byte \t\t at address " << addr << std::endl;
        return memory[addr];
    }
    else
    {
        out << std::hex << app->cpu->PC << "\tGet byte out of range: " << addr << std::endl;
        return 0;
    }
}

uint16_t MCD212::GetWord(const uint32_t& addr)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount++;
        return memory[addr + 0x400000] << 8 | memory[addr + 0x400001];
    }
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        out << std::hex << app->cpu->PC << "\tGet WORD register: " << addr << std::endl;
        return registers[addr-0x4FFFE0];
    }
    else if(addr < 0x4FFFDF)
    {
        if(addr != app->cpu->PC)
            out << std::hex << app->cpu->PC << "\tGet word \t\t at address " << addr << std::endl;
        return memory[addr] << 8 | memory[addr + 1];
    }
    else
    {
        out << std::hex << app->cpu->PC << "\tGet word out of range: " << addr << std::endl;
        return 0;
    }
}

uint32_t MCD212::GetLong(const uint32_t& addr)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount += 2;
        return memory[addr + 0x400000] << 24 | memory[addr + 0x400001] << 16 | memory[addr + 0x400002] << 8 | memory[addr + 0x400003];
    }
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        out << std::hex << app->cpu->PC << "\tGet LONG register: " << addr << std::endl;
        return registers[addr-0x4FFFE0] << 16 | registers[addr-0x4FFFDF];
    }
    else if(addr < 0x4FFFDF)
    {
        out << std::hex << app->cpu->PC << "\tGet long \t\t at address " << addr << std::endl;
        return memory[addr] << 24 | memory[addr + 1] << 16 | memory[addr + 2] << 8 | memory[addr + 3];
    }
    else
    {
        out << std::hex << app->cpu->PC << "\tGet long out of range: " << addr << std::endl;
        return 0;
    }
}

void MCD212::SetByte(const uint32_t& addr, const uint8_t& data)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        out << std::hex << app->cpu->PC << "\tSet BYTE register: " << addr << " value " << data << std::endl;
    }
    else if(addr <= 0x4FFFDF)
    {
        out << std::hex << app->cpu->PC << "\tSet byte " << std::to_string(data) << " \tat address " << addr << std::endl;
        memory[addr] = data;
    }
    else
    {
        out << std::hex << app->cpu->PC << "\tSet byte out of range: " << addr << " value " << std::to_string(data) << std::endl;
    }
}

void MCD212::SetWord(const uint32_t& addr, const uint16_t& data)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        out << std::hex << app->cpu->PC << "\tSet register: " << addr << " value " << data << std::endl;
    }
    else if(addr < 0x4FFFDF)
    {
        out << std::hex << app->cpu->PC << "\tSet word " << std::to_string(data) << " \tat address " << addr << std::endl;
        memory[addr] = data >> 8;
        memory[addr+1] = data;
    }
    else
    {
        out << std::hex << app->cpu->PC << "\tSet word out of range: " << addr << " value " << std::to_string(data) << std::endl;
    }
}

void MCD212::SetLong(const uint32_t& addr, const uint32_t& data)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        out << std::hex << app->cpu->PC << "\tSet LONG register: " << addr << " value " << data << std::endl;
    }
    else if(addr < 0x4FFFDF)
    {
        out << std::hex << app->cpu->PC << "\tSet long " << std::to_string(data) << " \tat address " << addr << std::endl;
        memory[addr]   = data >> 24;
        memory[addr+1] = (data & 0x00FF0000) >> 16;
        memory[addr+2] = (data & 0x0000FF00) >> 8;
        memory[addr+3] = data;
    }
    else
    {
        out << std::hex << app->cpu->PC << "\tSet long out of range: " << addr << " value " << std::to_string(data) << std::endl;
    }
}
