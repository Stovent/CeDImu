#include "MCD212.hpp"

#include <iomanip>

#include "../../utils.hpp"

uint8_t MCD212::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr <= 0x4FFFDF)
    {
        LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get byte at 0x" << std::setw(6) << std::setfill('0') << addr << " : (0x" << std::setw(8) << (int)memory[addr] << ") " << std::dec << (int)memory[addr] << std::endl; })
        return memory[addr];
    }

    if(addr <= 0x4FFFFF)
    {
        const uint8_t data = internalRegisters[addr-0x4FFFE0] & 0x00FF;
        LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << (int)data << std::endl; })
        if(addr == 0x4FFFE1 && flags & Trigger)
        {
            internalRegisters[CSR2R] &= 0x00FE; // clear BE bit on status read
        }
        return data;
    }

    LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get byte at 0x" << addr << " WARNING: out of range" << std::endl)
    return 0;
}

uint16_t MCD212::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount++;
        return memory[addr + 0x400000] << 8 | memory[addr + 0x400001];
    }

    if(addr < 0x4FFFDF)
    {
        const uint16_t data = memory[addr] << 8 | memory[addr + 1];
        LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get word at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int16_t)data << std::endl; })
        return data;
    }

    if(addr < 0x4FFFFF)
    {
        LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << internalRegisters[addr-0x4FFFE0] << " WARNING: unexpected word size" << std::endl; })
        return internalRegisters[addr-0x4FFFE0];
    }

    LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get word at 0x" << addr << " WARNING: out of range" << std::endl)
    return 0;
}

uint32_t MCD212::GetLong(const uint32_t addr, const uint8_t flags)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount += 2;
        return memory[addr + 0x400000] << 24 | memory[addr + 0x400001] << 16 | memory[addr + 0x400002] << 8 | memory[addr + 0x400003];
    }

    if(addr < 0x4FFFDF)
    {
        const uint32_t data = memory[addr] << 24 | memory[addr + 1] << 16 | memory[addr + 2] << 8 | memory[addr + 3];
        LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get long at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int32_t)data << std::endl; })
        return data;
    }

    if(addr < 0x4FFFFF)
    {
        LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << internalRegisters[addr-0x4FFFE0] << " WARNING: unexpected long size" << std::endl; })
        return internalRegisters[addr-0x4FFFE0];
    }

    LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get long at 0x" << addr << " WARNING: out of range" << std::endl)
    return 0;
}

void MCD212::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr <= 0x4FFFDF)
    {
        LOG(if(addr >= 0x400000) { out_dram << "WARNING: writing to System ROM ";} if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set byte at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << (int)data << ") " << std::dec << (int)data << std::endl; })
        memory[addr] = data;
    }
    else if(addr <= 0x4FFFFF)
    {
        LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << data << " WARNING: unexpected size byte" << std::endl; })
        internalRegisters[addr-0x4FFFE0] = data;
    }
    else
    {
        LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set byte at 0x" << addr << " : 0x" << data << " WARNING: out of range" << std::endl)
    }
}

void MCD212::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x4FFFDF)
    {
        LOG(if(addr >= 0x400000) { out_dram << "WARNING: writing to System ROM ";} if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set word at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int16_t)data << std::endl; })
        memory[addr] = data >> 8;
        memory[addr+1] = data;
    }
    else if(addr < 0x4FFFFF)
    {
        LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << data << std::endl; })
        internalRegisters[addr-0x4FFFE0] = data;
    }
    else
    {
        LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set word at 0x" << addr << " : 0x" << data << " WARNING: out of range" << std::endl)
    }
}

void MCD212::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    if(addr < 0x4FFFDF)
    {
        LOG(if(addr >= 0x400000) { out_dram << "WARNING: writing to System ROM ";} if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set long at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int32_t)data << std::endl; })
        memory[addr]   = data >> 24;
        memory[addr+1] = (data & 0x00FF0000) >> 16;
        memory[addr+2] = (data & 0x0000FF00) >> 8;
        memory[addr+3] = data;
    }
    else if(addr < 0x4FFFFF)
    {
        LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << data << " WARNING: unexpected size long" << std::endl; })
        internalRegisters[addr-0x4FFFE0] = data;
    }
    else
    {
        LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set long at 0x" << addr << " : 0x" << data << " WARNING: out of range" << std::endl)
    }
}
