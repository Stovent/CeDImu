#include "MCD212.hpp"
#include "../../common/utils.hpp"
#include "../../boards/Board.hpp"

#include <iomanip>

uint8_t MCD212::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr <= 0x4FFFDF)
    {
        const uint8_t data = memory[addr];
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet byte at 0x%06X : %d %d 0x%X", board->cpu.currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    if(addr == 0x4FFFE1)
    {
        const uint8_t data = registerCSR2R;
        if(flags & Trigger)
            registerCSR2R &= 0x06; // clear BE bit on status read
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet CSR2R register at 0x%06X : 0x%X", board->cpu.currentPC, addr, data); })
        return data;
    }

    if(addr == 0x4FFFF1)
    {
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet CSR1R register at 0x%06X : 0x%X", board->cpu.currentPC, addr, registerCSR1R); })
        return registerCSR1R;
    }

    LOG(fprintf(out_dram, "%6X\tGet byte OUT OF RANGE at 0x%X\n", board->cpu.currentPC, addr);)
    return 0;
}

uint16_t MCD212::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(memorySwapCount < 4 && flags & Trigger)
    {
        memorySwapCount++;
        return memory[addr + 0x400000] << 8 | memory[addr + 0x400001];
    }

    if(addr < 0x4FFFDF)
    {
        const uint16_t data = memory[addr] << 8 | memory[addr + 1];
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet word at 0x%06X : %d %d 0x%X", board->cpu.currentPC, addr, (int16_t)data, data, data); })
        return data;
    }

    if(addr == 0x4FFFE0) // word size: MSB is 0, LSB is the register
    {
        const uint16_t data = registerCSR2R;
        if(flags & Trigger)
            registerCSR2R &= 0x06; // clear BE bit on status read
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet CSR2R register at 0x%06X : 0x%X", board->cpu.currentPC, addr, data); })
        return data;
    }

    if(addr == 0x4FFFF0) // word size: MSB is 0, LSB is the register
    {
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet CSR1R register at 0x%06X : 0x%X", board->cpu.currentPC, addr, registerCSR1R); })
        return registerCSR1R;
    }

    LOG(fprintf(out_dram, "%6X\tGet word OUT OF RANGE at 0x%X\n", board->cpu.currentPC, addr);)
    return 0;
}

uint32_t MCD212::GetLong(const uint32_t addr, const uint8_t flags)
{
    if(memorySwapCount < 4 && flags & Trigger)
    {
        memorySwapCount += 2;
        return memory[addr + 0x400000] << 24 | memory[addr + 0x400001] << 16 | memory[addr + 0x400002] << 8 | memory[addr + 0x400003];
    }

    if(addr < 0x4FFFDF)
    {
        const uint32_t data = memory[addr] << 24 | memory[addr + 1] << 16 | memory[addr + 2] << 8 | memory[addr + 3];
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet long at 0x%06X : %d %d 0x%X", board->cpu.currentPC, addr, (int32_t)data, data, data); })
        return data;
    }

    LOG(fprintf(out_dram, "%6X\tGet long OUT OF RANGE at 0x%X\n", board->cpu.currentPC, addr);)
    return 0;
}

void MCD212::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x400000)
    {
        memory[addr] = data;
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tSet byte at 0x%06X : %d %d 0x%X", board->cpu.currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    if(addr >= 0x4FFFE0 || addr <= 0x4FFFFF)
    {
        internalRegisters[addr-0x4FFFE0] &= 0xFF00;
        internalRegisters[addr-0x4FFFE0] |= data;
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tSet byte register at 0x%06X : 0x%X", board->cpu.currentPC, addr, data); })
        return;
    }

    LOG(fprintf(out_dram, "%6X\tSet byte OUT OF RANGE at 0x%X : %d %d 0x%X\n", board->cpu.currentPC, addr, (int8_t)data, data, data);)
}

void MCD212::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x400000)
    {
        memory[addr] = data >> 8;
        memory[addr+1] = data;
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tSet word at 0x%06X : %d %d 0x%X", board->cpu.currentPC, addr, (int16_t)data, data, data); })
        return;
    }

    if(addr >= 0x4FFFE0 || addr <= 0x4FFFFF)
    {
        internalRegisters[addr-0x4FFFE0] = data;
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tSet word register at 0x%06X : 0x%X", board->cpu.currentPC, addr, data); })
        return;
    }

    LOG(fprintf(out_dram, "%6X\tSet word OUT OF RANGE at 0x%X : %d %d 0x%X\n", board->cpu.currentPC, addr, (int16_t)data, data, data);)
}

void MCD212::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    if(addr < 0x400000)
    {
        memory[addr]   = data >> 24;
        memory[addr+1] = (data & 0x00FF0000) >> 16;
        memory[addr+2] = (data & 0x0000FF00) >> 8;
        memory[addr+3] = data;
        LOG(if(flags & Log) { fprintf(out_dram, "%6X\tSet long at 0x%06X : %d %d 0x%X", board->cpu.currentPC, addr, (int32_t)data, data, data); })
        return;
    }

    LOG(fprintf(out_dram, "%6X\tSet long OUT OF RANGE at 0x%X : %d %d 0x%X\n", board->cpu.currentPC, addr, (int32_t)data, data, data);)
}
