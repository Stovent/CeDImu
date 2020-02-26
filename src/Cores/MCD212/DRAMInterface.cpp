#include "../../utils.hpp"
#include "MCD212.hpp"

uint8_t MCD212::GetByte(const uint32_t& addr)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        LOG(out, std::hex << app->cpu->currentPC << "\tGet internal register: 0x" << addr << " : 0x" << (internalRegisters[addr-0x4FFFE0] & 0x00FF))
        const uint8_t ret = internalRegisters[addr-0x4FFFE0] & 0x00FF;
        if(addr == 0x4FFFE1)
        {
            internalRegisters[CSR2R] &= 0x00FE; // clear BE bit on status read
        }
        return ret;
    }
    else if(addr <= 0x4FFFDF)
    {
        LOG(out, std::hex << app->cpu->currentPC << "\tGet byte at address 0x" << addr << " : " << std::dec << (int)((int8_t)memory[addr]) << " (0x" << std::hex << (int)memory[addr] << ")")
        return memory[addr];
    }
    else
    {
        LOG(out, std::hex << app->cpu->currentPC << "\tGet byte out of range: 0x" << addr)
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
        LOG(out, std::hex << app->cpu->currentPC << "\tWARNING: get WORD internal register: 0x" << addr)
        return internalRegisters[addr-0x4FFFE0];
    }
    else if(addr < 0x4FFFDF)
    {
        LOG(if(addr != app->cpu->currentPC) out, std::hex << app->cpu->currentPC << "\tGet word at address 0x" << addr << " : " << std::dec << (int16_t)(memory[addr] << 8 | memory[addr + 1]) << " (0x" << std::hex << (memory[addr] << 8 | memory[addr + 1]) << ")")
        return memory[addr] << 8 | memory[addr + 1];
    }
    else
    {
        LOG(out, std::hex << app->cpu->currentPC << "\tGet word out of range: 0x" << addr)
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
        LOG(out, std::hex << app->cpu->currentPC << "\tWARNING: get LONG internal register: 0x" << addr)
        return internalRegisters[addr-0x4FFFE0] << 16 | internalRegisters[addr-0x4FFFDF];
    }
    else if(addr < 0x4FFFDF)
    {
        LOG(if(!isCA) out, std::hex << app->cpu->currentPC << "\tGet long at address 0x" << addr << " : " << std::dec << (int32_t)(memory[addr] << 24 | memory[addr + 1] << 16 | memory[addr + 2] << 8 | memory[addr + 3]) << " (0x" << std::hex << (memory[addr] << 24 | memory[addr + 1] << 16 | memory[addr + 2] << 8 | memory[addr + 3]) << ")")
        return memory[addr] << 24 | memory[addr + 1] << 16 | memory[addr + 2] << 8 | memory[addr + 3];
    }
    else
    {
        LOG(out, std::hex << app->cpu->currentPC << "\tGet long out of range: 0x" << addr)
        return 0;
    }
}

void MCD212::SetByte(const uint32_t& addr, const uint8_t& data)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        LOG(out, std::hex << app->cpu->currentPC << "\tWARNING: set BYTE internal register: 0x" << addr << " value 0x" << data)
        internalRegisters[addr-0x4FFFE0] = data;
    }
    else if(addr <= 0x4FFFDF)
    {
        LOG(if(addr >= 0x400000) out << "WARNING: writing to System ROM "; out, std::hex << app->cpu->currentPC << "\tSet byte " << std::dec << (int)data << " (0x" << std::hex << (int)data << ") \tat address 0x" << addr)
        memory[addr] = data;
    }
    else
    {
        LOG(out, std::hex << app->cpu->currentPC << "\tSet byte out of range: 0x" << addr << " value " << std::dec << data)
    }
}

void MCD212::SetWord(const uint32_t& addr, const uint16_t& data)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        LOG(out, std::hex << app->cpu->currentPC << "\tSet internal register: 0x" << addr << " value 0x" << data)
        internalRegisters[addr-0x4FFFE0] = data;
    }
    else if(addr <= 0x4FFFDF)
    {
        LOG(if(addr >= 0x400000) out << "WARNING: writing to System ROM "; out, std::hex << app->cpu->currentPC << "\tSet word " << std::dec << data << " (0x" << std::hex << data << ") \tat address 0x" << addr)
        memory[addr] = data >> 8;
        memory[addr+1] = data;
    }
    else
    {
        LOG(out, std::hex << app->cpu->currentPC << "\tSet word out of range: 0x" << addr << " value " << std::dec << data)
    }
}

void MCD212::SetLong(const uint32_t& addr, const uint32_t& data)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        LOG(out, std::hex << app->cpu->currentPC << "\tWARNING: set LONG internal register: 0x" << addr << " value 0x" << data)
        internalRegisters[addr-0x4FFFE0] = data;
    }
    else if(addr <= 0x4FFFDF)
    {
        LOG(if(addr >= 0x400000) out << "WARNING: writing to System ROM "; out, std::hex << app->cpu->currentPC << "\tSet long " << std::dec << data << " (0x" << std::hex << data << ") \tat address 0x" << addr)
        memory[addr]   = data >> 24;
        memory[addr+1] = (data & 0x00FF0000) >> 16;
        memory[addr+2] = (data & 0x0000FF00) >> 8;
        memory[addr+3] = data;
    }
    else
    {
        LOG(out, std::hex << app->cpu->currentPC << "\tSet long out of range: 0x" << addr << " value " << std::dec << data)
    }
}
