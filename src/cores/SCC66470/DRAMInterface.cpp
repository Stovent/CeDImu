#include "SCC66470.hpp"

uint8_t SCC66470::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(isMaster)
    {
        if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00))
        {
            return memory[addr];
        }

        if(addr >= 0x1FFFE0 && addr < 0x200000)
        {
            return internalRegisters[addr - 0x1FFFE0];
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            return memory[addr];
        }

        if(addr >= 0x1FFFC0 && addr < 0x1FFFE0)
        {
            return internalRegisters[addr - 0x1FFFC0];
        }
    }

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
            return memory[addr] << 8 | memory[addr+1];
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            return memory[addr] << 8 | memory[addr+1];
        }
    }

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
            return memory[addr] << 24 | memory[addr+1] << 16 | memory[addr+2] << 8 | memory[addr+3];
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            return memory[addr] << 24 | memory[addr+1] << 16 | memory[addr+2] << 8 | memory[addr+3];
        }
    }

    return 0;
}

void SCC66470::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(isMaster)
    {
        if(addr < 0x080000)
        {
            memory[addr] = data;
        }

        if(addr >= 0x1FFFE0 && addr < 0x200000)
        {
            internalRegisters[addr - 0x1FFFE0] &= 0xFF00;
            internalRegisters[addr - 0x1FFFE0] |= data;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            memory[addr] = data;
        }

        if(addr >= 0x1FFFC0 && addr < 0x1FFFE0)
        {
            internalRegisters[addr - 0x1FFFC0] &= 0xFF00;
            internalRegisters[addr - 0x1FFFC0] |= data;
        }
    }
}

void SCC66470::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(isMaster)
    {
        if(addr < 0x080000)
        {
            memory[addr] = (data & 0xFF00) >> 8;
            memory[addr + 1] = data & 0x00FF;
        }

        if(addr >= 0x1FFFE0 && addr < 0x200000)
        {
            internalRegisters[addr - 0x1FFFE0] = data;
        }
    }
    else
    {
        if(addr >= 0x080000 && addr < 0x100000)
        {
            memory[addr] = (data & 0xFF00) >> 8;
            memory[addr + 1] = data & 0x00FF;
        }

        if(addr >= 0x1FFFC0 && addr < 0x1FFFE0)
        {
            internalRegisters[addr - 0x1FFFC0] = data;
        }
    }
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
        }
    }
}
