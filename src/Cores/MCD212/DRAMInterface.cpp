#include "../../utils.hpp"
#include "MCD212.hpp"

uint8_t MCD212::GetByte(const uint32_t& addr)
{
    if(addr >= 0x80 && addr <= 0xFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tWARNING: get BYTE register: 0x" << addr << std::endl;
#endif // DEBUG
        return registers[addr-0x80];
    }
    else if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tGet internal register: 0x" << addr << std::endl;
#endif // DEBUG
        uint8_t ret = registers[addr-0x4FFFE0] & 0x00FF;
        if(addr == 0x4FFFE1)
        {
            registers[CSR2R] &= 0x00FE;
        }
        return ret;
    }
    else if(addr <= 0x4FFFDF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tGet byte \t\t at address 0x" << addr << std::endl;
#endif // DEBUG
        return memory[addr];
    }
    else
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tGet byte out of range: 0x" << addr << std::endl;
#endif // DEBUG
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
    if(addr >= 0x80 && addr <= 0xFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tWARNING: get WORD register: 0x" << addr << std::endl;
#endif // DEBUG
        return registers[addr-0x80];
    }
    else if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tWARNING: get WORD internal register: 0x" << addr << std::endl;
#endif // DEBUG
        return registers[addr-0x4FFFE0];
    }
    else if(addr < 0x4FFFDF)
    {
#ifdef DEBUG
        if(addr != app->cpu->currentPC)
            out << std::hex << app->cpu->currentPC << "\tGet word \t\t at address 0x" << addr << std::endl;
#endif // DEBUG
        return memory[addr] << 8 | memory[addr + 1];
    }
    else
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tGet word out of range: 0x" << addr << std::endl;
#endif // DEBUG
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
    if(addr >= 0x80 && addr <= 0xFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tGet register: 0x" << addr << std::endl;
#endif // DEBUG
        return registers[addr-0x80];
    }
    else if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tWARNING: get LONG internal register: 0x" << addr << std::endl;
#endif // DEBUG
        return registers[addr-0x4FFFE0] << 16 | registers[addr-0x4FFFDF];
    }
    else if(addr < 0x4FFFDF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tGet long \t\t at address 0x" << addr << std::endl;
#endif // DEBUG
        return memory[addr] << 24 | memory[addr + 1] << 16 | memory[addr + 2] << 8 | memory[addr + 3];
    }
    else
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tGet long out of range: 0x" << addr << std::endl;
#endif // DEBUG
        return 0;
    }
}

void MCD212::SetByte(const uint32_t& addr, const uint8_t& data)
{
    if(addr >= 0x80 && addr <= 0xFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tWARNING: set BYTE register: 0x" << addr << " value 0x" << data << std::endl;
#endif // DEBUG
        registers[addr-0x80] = data;
    }
    else if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tWARNING: set BYTE internal register: 0x" << addr << " value 0x" << data << std::endl;
#endif // DEBUG
    }
    else if(addr <= 0x4FFFDF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tSet byte " << std::to_string(data) << " \tat address 0x" << addr << std::endl;
#endif // DEBUG
        memory[addr] = data;
    }
    else
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tSet byte out of range: 0x" << addr << " value " << std::dec << data << std::endl;
#endif // DEBUG
    }
}

void MCD212::SetWord(const uint32_t& addr, const uint16_t& data)
{
    if(addr >= 0x80 && addr <= 0xFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tWARNING: set WORD register: 0x" << addr << " value 0x" << data << std::endl;
#endif // DEBUG
        registers[addr-0x80] = data;
    }
    else if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tSet internal register: 0x" << addr << " value 0x" << data << std::endl;
#endif // DEBUG
    }
    else if(addr < 0x4FFFDF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tSet word " << std::to_string(data) << " \tat address 0x" << addr << std::endl;
#endif // DEBUG
        memory[addr] = data >> 8;
        memory[addr+1] = data;
    }
    else
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tSet word out of range: 0x" << addr << " value " << std::dec << data << std::endl;
#endif // DEBUG
    }
}

void MCD212::SetLong(const uint32_t& addr, const uint32_t& data)
{
    if(addr >= 0x80 && addr <= 0xFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tSet register: 0x" << addr << " value 0x" << data << std::endl;
#endif // DEBUG
        registers[addr-0x80] = data;
    }
    else if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tWARNING: set LONG internal register: 0x" << addr << " value 0x" << data << std::endl;
#endif // DEBUG
    }
    else if(addr < 0x4FFFDF)
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tSet long " << std::to_string(data) << " \tat address 0x" << addr << std::endl;
#endif // DEBUG
        memory[addr]   = data >> 24;
        memory[addr+1] = (data & 0x00FF0000) >> 16;
        memory[addr+2] = (data & 0x0000FF00) >> 8;
        memory[addr+3] = data;
    }
    else
    {
#ifdef DEBUG
        out << std::hex << app->cpu->currentPC << "\tSet long out of range: 0x" << addr << " value " << std::dec << data << std::endl;
#endif // DEBUG
    }
}
