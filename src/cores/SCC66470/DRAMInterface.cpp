#include "SCC66470.hpp"
#include "../../Boards/Board.hpp"
#include "../../utils.hpp"

#include <iomanip>

uint8_t SCC66470::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(isMaster)
    {
        if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00))
        {
            const uint8_t data = memory[addr];
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get byte at 0x" << std::setw(6) << std::setfill('0') << addr << " : (0x" << std::setw(8) << (int)data << ") " << std::dec << (int)data << std::endl; })
            return data;
        }

        if(addr >= 0x1FFFE0 && addr < 0x200000)
        {
            const uint8_t data = internalRegisters[addr - 0x1FFFE0];
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << (int)data << std::endl; })
            return data;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            const uint8_t data = memory[addr];
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get byte at 0x" << std::setw(6) << std::setfill('0') << addr << " : (0x" << std::setw(8) << (int)data << ") " << std::dec << (int)data << std::endl; })
            return data;
        }

        if(addr >= 0x1FFFC0 && addr < 0x1FFFE0)
        {
            const uint8_t data = internalRegisters[addr - 0x1FFFC0];
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << (int)data << std::endl; })
            return data;
        }
    }

    LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get byte OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

uint16_t SCC66470::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount += 1;
        return memory[addr+0x180000] << 8 | memory[addr+0x180001];

    }

    if(isMaster)
    {
        if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00))
        {
            const uint16_t data = memory[addr] << 8 | memory[addr+1];
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get word at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int16_t)data << std::endl; })
            return data;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            const uint16_t data = memory[addr] << 8 | memory[addr+1];
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get word at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int16_t)data << std::endl; })
            return data;
        }
    }

    LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get word OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

uint32_t SCC66470::GetLong(const uint32_t addr, const uint8_t flags)
{
    if(memorySwapCount < 4)
    {
        memorySwapCount += 2;
        return memory[addr+0x180000] << 24 | memory[addr+0x180001] << 16 | memory[addr+0x180002] << 8 | memory[addr+0x180003];

    }

    if(isMaster)
    {
        if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00))
        {
            const uint32_t data = memory[addr] << 24 | memory[addr+1] << 16 | memory[addr+2] << 8 | memory[addr+3];
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get long at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int32_t)data << std::endl; })
            return data;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            const uint32_t data = memory[addr] << 24 | memory[addr+1] << 16 | memory[addr+2] << 8 | memory[addr+3];
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get long at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int32_t)data << std::endl; })
            return data;
        }
    }

    LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Get long OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

void SCC66470::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(isMaster)
    {
        if(addr < 0x080000)
        {
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set byte at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << (int)data << ") " << std::dec << (int)data << std::endl; })
            memory[addr] = data;
            return;
        }

        if(addr >= 0x1FFFE0 && addr < 0x200000)
        {
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set byte register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << (int)data << std::endl; })
            internalRegisters[addr - 0x1FFFE0] &= 0xFF00;
            internalRegisters[addr - 0x1FFFE0] |= data;
            return;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set byte at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << (int)data << ") " << std::dec << (int)data << std::endl; })
            memory[addr] = data;
            return;
        }

        if(addr >= 0x1FFFC0 && addr < 0x1FFFE0)
        {
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set byte register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << (int)data << std::endl; })
            internalRegisters[addr - 0x1FFFC0] &= 0xFF00;
            internalRegisters[addr - 0x1FFFC0] |= data;
            return;
        }
    }

    LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set byte OUT OF RANGE at 0x" << addr << " : 0x" << (int)data << std::endl)
}

void SCC66470::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(isMaster)
    {
        if(addr < 0x080000)
        {
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set word at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int16_t)data << std::endl; })
            memory[addr] = (data & 0xFF00) >> 8;
            memory[addr + 1] = data & 0x00FF;
            return;
        }

        if(addr >= 0x1FFFE0 && addr < 0x200000)
        {
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << data << std::endl; })
            internalRegisters[addr - 0x1FFFE0] = data;
            return;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set word at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int16_t)data << std::endl; })
            memory[addr] = (data & 0xFF00) >> 8;
            memory[addr + 1] = data & 0x00FF;
            return;
        }

        if(addr >= 0x1FFFC0 && addr < 0x1FFFE0)
        {
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set register at 0x" << std::setw(6) << std::setfill('0') << addr << " : 0x" << data << std::endl; })
            internalRegisters[addr - 0x1FFFC0] = data;
            return;
        }
    }

    LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set word OUT OF RANGE at 0x" << addr << " : 0x" << data << std::endl)
}

void SCC66470::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    if(isMaster)
    {
        if(addr < 0x080000)
        {
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set long at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int32_t)data << std::endl; })
            memory[addr]     = (data & 0xFF000000) >> 24;
            memory[addr + 1] = (data & 0x00FF0000) >> 16;
            memory[addr + 2] = (data & 0x0000FF00) >> 8;
            memory[addr + 3] = (data & 0x000000FF);
            return;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            LOG(if(flags & Log) { out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set long at 0x" << std::setfill('0') << std::setw(6) << addr << " : (0x" << std::setw(8) << data << ") " << std::dec << (int32_t)data << std::endl; })
            memory[addr]     = (data & 0xFF000000) >> 24;
            memory[addr + 1] = (data & 0x00FF0000) >> 16;
            memory[addr + 2] = (data & 0x0000FF00) >> 8;
            memory[addr + 3] = (data & 0x000000FF);
            return;
        }
    }

    LOG(out_dram << std::setw(6) << std::hex << board->cpu->currentPC << " Set long OUT OF RANGE at 0x" << addr << " : 0x" << data << std::endl)
}
