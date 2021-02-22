#include "SCC66470.hpp"
#include "../../boards/Board.hpp"
#include "../../common/utils.hpp"

uint8_t SCC66470::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(isMaster)
    {
        if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00))
        {
            const uint8_t data = memory[addr];
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tGet byte at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int8_t)data, data, data); })
            return data;
        }

        if(addr == 0x1FFFE1)
        {
            const uint8_t data = registerCSR;
            if(flags & Trigger)
                registerCSR &= 0xE6; // clear BE bit on status read
            LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet CSR register at 0x%06X : 0x%X", board->cpu.currentPC, addr, data); })
            return data;
        }

        if(addr == 0x1FFFF2 || addr == 0x1FFFF3)
        {
            uint8_t data;
            if(isEven(addr))
                data = internalRegisters[SB] >> 8;
            else
                data = internalRegisters[SB];
            LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet register B register at 0x%06X : 0x%X", board->cpu.currentPC, addr, data); })
            return data;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            const uint8_t data = memory[addr];
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tGet byte at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int8_t)data, data, data); })
            return data;
        }

        if(addr == 0x1FFFC1)
        {
            const uint8_t data = registerCSR;
            if(flags & Trigger)
                registerCSR &= 0xE6; // clear BE bit on status read
            LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet CSR register at 0x%06X : 0x%X", board->cpu.currentPC, addr, data); })
            return data;
        }

        if(addr == 0x1FFFD2 || addr == 0x1FFFD3)
        {
            uint8_t data;
            if(isEven(addr))
                data = internalRegisters[SB] >> 8;
            else
                data = internalRegisters[SB];
            LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet register B register at 0x%06X : 0x%X", board->cpu.currentPC, addr, data); })
            return data;
        }
    }

    LOG(if(flags & Log) { fprintf(out_dram, "%X\tGet byte OUT OF RANGE at 0x%X\n", board->cpu.currentPC, addr); })
    return 0;
}

uint16_t SCC66470::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(memorySwapCount < 4 && flags & Trigger)
    {
        memorySwapCount += 1;
        return memory[addr+0x180000] << 8 | memory[addr+0x180001];
    }

    if(isMaster)
    {
        if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00))
        {
            const uint16_t data = memory[addr] << 8 | memory[addr+1];
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tGet word at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int16_t)data, data, data); })
            return data;
        }

        if(addr == 0x1FFFE0)
        {
            const uint8_t data = registerCSR;
            if(flags & Trigger)
                registerCSR &= 0xE6; // clear BE bit on status read
            LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet CSR register at 0x%06X : 0x%X", board->cpu.currentPC, addr, data); })
            return data;
        }

        if(addr == 0x1FFFF2)
        {
            LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet register B at 0x%06X : 0x%X", board->cpu.currentPC, addr, registerB); })
            return registerB;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            const uint16_t data = memory[addr] << 8 | memory[addr+1];
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tGet word at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int16_t)data, data, data); })
            return data;
        }

        if(addr == 0x1FFFC0)
        {
            const uint8_t data = registerCSR;
            if(flags & Trigger)
                registerCSR &= 0xE6; // clear BE bit on status read
            LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet CSR register at 0x%06X : 0x%X", board->cpu.currentPC, addr, data); })
            return data;
        }

        if(addr == 0x1FFFD2)
        {
            LOG(if(flags & Log) { fprintf(out_dram, "%6X\tGet register B at 0x%06X : 0x%X", board->cpu.currentPC, addr, registerB); })
            return registerB;
        }
    }

    LOG(if(flags & Log) { fprintf(out_dram, "%X\tGet word OUT OF RANGE at 0x%X\n", board->cpu.currentPC, addr); })
    return 0;
}

uint32_t SCC66470::GetLong(const uint32_t addr, const uint8_t flags)
{
    if(memorySwapCount < 4 && flags & Trigger)
    {
        memorySwapCount += 2;
        return memory[addr+0x180000] << 24 | memory[addr+0x180001] << 16 | memory[addr+0x180002] << 8 | memory[addr+0x180003];
    }

    if(isMaster)
    {
        if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00))
        {
            const uint32_t data = memory[addr] << 24 | memory[addr+1] << 16 | memory[addr+2] << 8 | memory[addr+3];
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tGet long at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int32_t)data, data, data); })
            return data;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            const uint32_t data = memory[addr] << 24 | memory[addr+1] << 16 | memory[addr+2] << 8 | memory[addr+3];
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tGet long at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int32_t)data, data, data); })
            return data;
        }
    }

    LOG(if(flags & Log) { fprintf(out_dram, "%X\tGet long OUT OF RANGE at 0x%X\n", board->cpu.currentPC, addr); })
    return 0;
}

void SCC66470::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(isMaster)
    {
        if(addr < 0x080000)
        {
            memory[addr] = data;
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet byte at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int8_t)data, data, data); })
            return;
        }

        if(addr >= 0x1FFFE0 && addr < 0x200000)
        {
            if(isEven(addr))
            {
                internalRegisters[addr - 0x1FFFE0] &= 0x00FF;
                internalRegisters[addr - 0x1FFFE0] |= (uint16_t)data << 8;
            }
            else
            {
                internalRegisters[addr - 0x1FFFE0] &= 0xFF00;
                internalRegisters[addr - 0x1FFFE0] |= data;
            }
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet byte register at 0x%X: 0x%X\n", board->cpu.currentPC, addr, data); })
            return;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            memory[addr] = data;
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet byte at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int8_t)data, data, data); })
            return;
        }

        if(addr >= 0x1FFFC0 && addr < 0x1FFFE0)
        {
            if(isEven(addr))
            {
                internalRegisters[addr - 0x1FFFC0] &= 0x00FF;
                internalRegisters[addr - 0x1FFFC0] |= (uint16_t)data << 8;
            }
            else
            {
                internalRegisters[addr - 0x1FFFC0] &= 0xFF00;
                internalRegisters[addr - 0x1FFFC0] |= data;
            }
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet byte register at 0x%X: 0x%X\n", board->cpu.currentPC, addr, data); })
            return;
        }
    }

    LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet byte OUT OF RANGE at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int8_t)data, data, data); })
}

void SCC66470::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(isMaster)
    {
        if(addr < 0x080000)
        {
            memory[addr] = (data & 0xFF00) >> 8;
            memory[addr + 1] = data & 0x00FF;
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet word at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int16_t)data, data, data); })
            return;
        }

        if(addr >= 0x1FFFE0 && addr < 0x200000)
        {
            internalRegisters[addr - 0x1FFFE0] = data;
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet word register at 0x%X: 0x%X\n", board->cpu.currentPC, addr, data); })
            return;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            memory[addr] = (data & 0xFF00) >> 8;
            memory[addr + 1] = data & 0x00FF;
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet word at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int16_t)data, data, data); })
            return;
        }

        if(addr >= 0x1FFFC0 && addr < 0x1FFFE0)
        {
            internalRegisters[addr - 0x1FFFC0] = data;
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet word register at 0x%X: 0x%X\n", board->cpu.currentPC, addr, data); })
            return;
        }
    }

    LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet word OUT OF RANGE at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int16_t)data, data, data); })
}

void SCC66470::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    if(isMaster)
    {
        if(addr < 0x080000)
        {
            memory[addr]     = (data & 0xFF000000) >> 24;
            memory[addr + 1] = (data & 0x00FF0000) >> 16;
            memory[addr + 2] = (data & 0x0000FF00) >> 8;
            memory[addr + 3] = (data & 0x000000FF);
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet long at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int32_t)data, data, data); })
            return;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            memory[addr]     = (data & 0xFF000000) >> 24;
            memory[addr + 1] = (data & 0x00FF0000) >> 16;
            memory[addr + 2] = (data & 0x0000FF00) >> 8;
            memory[addr + 3] = (data & 0x000000FF);
            LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet long at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int32_t)data, data, data); })
            return;
        }
    }

    LOG(if(flags & Log) { fprintf(out_dram, "%X\tSet long OUT OF RANGE at 0x%X: %d %d 0x%X\n", board->cpu.currentPC, addr, (int32_t)data, data, data); })
}
