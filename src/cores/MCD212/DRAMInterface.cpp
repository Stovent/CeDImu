#include "MCD212.hpp"

#include <iomanip>

#include "../../utils.hpp"

uint8_t MCD212::GetByteNoDebug(const uint32_t addr)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        return internalRegisters[addr-0x4FFFE0] & 0x00FF;
    }
    else if(addr <= 0x4FFFDF)
    {
        return memory[addr];
    }
    else
    {
        return 0;
    }
}

uint16_t MCD212::GetWordNoDebug(const uint32_t addr)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        return internalRegisters[addr-0x4FFFE0];
    }
    else if(addr <= 0x4FFFDF)
    {
        return memory[addr] << 8 | memory[addr + 1];
    }
    else
    {
        return 0;
    }
}

uint32_t MCD212::GetLongNoDebug(const uint32_t addr)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        return internalRegisters[addr-0x4FFFE0] << 16 | internalRegisters[addr-0x4FFFDF];
    }
    else if(addr <= 0x4FFFDF)
    {
        return memory[addr] << 24 | memory[addr + 1] << 16 | memory[addr + 2] << 8 | memory[addr + 3];
    }
    else
    {
        return 0;
    }
}

uint8_t MCD212::GetByte(const uint32_t addr)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Get register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << (internalRegisters[addr-0x4FFFE0] & 0x00FF) << std::endl)
        const uint8_t ret = internalRegisters[addr-0x4FFFE0] & 0x00FF;
        if(addr == 0x4FFFE1)
        {
            internalRegisters[CSR2R] &= 0x00FE; // clear BE bit on status read
        }
        return ret;
    }
    else if(addr <= 0x4FFFDF)
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Get byte at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << (int)memory[addr] << ") " << std::dec << (int)memory[addr] << std::endl)
        return memory[addr];
    }
    else
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Get byte out of range: 0x" << addr)
        return 0;
    }
}

uint16_t MCD212::GetWord(const uint32_t addr)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount++;
        return memory[addr + 0x400000] << 8 | memory[addr + 0x400001];
    }

    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Get register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << internalRegisters[addr-0x4FFFE0] << " WARNING: unexpected word size" << std::endl)
        return internalRegisters[addr-0x4FFFE0];
    }
    else if(addr < 0x4FFFDF)
    {
        const uint16_t data = memory[addr] << 8 | memory[addr + 1];
        LOG(if(addr != app->cpu->currentPC) out << std::setw(6) << std::hex << app->cpu->currentPC << " Get word at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int)data << std::endl)
        return data;
    }
    else
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Get word out of range: 0x" << addr)
        return 0;
    }
}

uint32_t MCD212::GetLong(const uint32_t addr)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount += 2;
        return memory[addr + 0x400000] << 24 | memory[addr + 0x400001] << 16 | memory[addr + 0x400002] << 8 | memory[addr + 0x400003];
    }

    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Get register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << internalRegisters[addr-0x4FFFE0] << " WARNING: unexpected long size" << std::endl)
        return internalRegisters[addr-0x4FFFE0] << 16 | internalRegisters[addr-0x4FFFDF];
    }
    else if(addr < 0x4FFFDF)
    {
        const uint32_t data = memory[addr] << 24 | memory[addr + 1] << 16 | memory[addr + 2] << 8 | memory[addr + 3];
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Get long at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int)data << std::endl)
        return data;
    }
    else
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Get long out of range: 0x" << addr)
        return 0;
    }
}

void MCD212::SetByte(const uint32_t addr, const uint8_t data)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Set register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << data << " WARNING: unexpected size byte" << std::endl)
        internalRegisters[addr-0x4FFFE0] = data;
    }
    else if(addr <= 0x4FFFDF)
    {
        LOG(if(addr >= 0x400000) out << "WARNING: writing to System ROM "; out << std::setw(6) << std::hex << app->cpu->currentPC << " Set byte at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << (int)data << ") " << std::dec << (int)data << std::endl)
        memory[addr] = data;
    }
    else
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Set byte out of range at 0x" << addr << " : 0x" << data << std::endl)
    }
}

void MCD212::SetWord(const uint32_t addr, const uint16_t data)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Set register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << data << std::endl)
        internalRegisters[addr-0x4FFFE0] = data;
    }
    else if(addr <= 0x4FFFDF)
    {
        LOG(if(addr >= 0x400000) out << "WARNING: writing to System ROM "; out << std::setw(6) << std::hex << app->cpu->currentPC << " Set word at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int)data << std::endl)
        memory[addr] = data >> 8;
        memory[addr+1] = data;
    }
    else
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Set word out of range at 0x" << addr << " : 0x" << data << std::endl)
    }
}

void MCD212::SetLong(const uint32_t addr, const uint32_t data)
{
    if(addr >= 0x4FFFE0 && addr <= 0x4FFFFF)
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Set register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << data << " WARNING: unexpected size long" << std::endl)
        internalRegisters[addr-0x4FFFE0] = data;
    }
    else if(addr <= 0x4FFFDF)
    {
        LOG(if(addr >= 0x400000) out << "WARNING: writing to System ROM "; out << std::setw(6) << std::hex << app->cpu->currentPC << " Set long at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int)data << std::endl)
        memory[addr]   = data >> 24;
        memory[addr+1] = (data & 0x00FF0000) >> 16;
        memory[addr+2] = (data & 0x0000FF00) >> 8;
        memory[addr+3] = data;
    }
    else
    {
        LOG(out << std::setw(6) << std::hex << app->cpu->currentPC << " Set long out of range at 0x" << addr << " : 0x" << data << std::endl)
    }
}
