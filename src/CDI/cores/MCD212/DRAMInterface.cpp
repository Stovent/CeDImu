#include "MCD212.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../common/utils.hpp"

uint8_t MCD212::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x400000)
    {
        const uint8_t data = memory[addr];
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::RAM, "Get", "Byte", cdi.board->cpu.currentPC, addr, data}); })
        return data;
    }

    if(addr < 0x4FFC00)
    {
        const uint8_t data = BIOS[addr - 0x400000];
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::BIOS, "Get", "Byte", cdi.board->cpu.currentPC, addr, data}); })
        return data;
    }

    if(addr == 0x4FFFE1)
    {
        const uint8_t data = registerCSR2R;
        if(flags & Trigger)
            registerCSR2R = 0; // clear IT1, IT2 and BE bits on status read
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::VDSC, "Get", "Byte", cdi.board->cpu.currentPC, addr, data}); })
        return data;
    }

    if(addr == 0x4FFFF1)
    {
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::VDSC, "Get", "Byte", cdi.board->cpu.currentPC, addr, registerCSR1R}); })
        return registerCSR1R;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Byte", cdi.board->cpu.currentPC, addr, 0}); })
    return 0;
}

uint16_t MCD212::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(memorySwapCount < 4 && flags & Trigger)
    {
        memorySwapCount++;
        return (uint16_t)BIOS[addr] << 8 | BIOS[addr + 1];
    }

    if(addr < 0x400000)
    {
        const uint16_t data = (uint16_t)memory[addr] << 8 | memory[addr + 1];
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::RAM, "Get", "Word", cdi.board->cpu.currentPC, addr, data}); })
        return data;
    }

    if(addr < 0x4FFC00)
    {
        const uint16_t data = (uint16_t)BIOS[addr - 0x400000] << 8 | BIOS[addr - 0x3FFFFF];
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::BIOS, "Get", "Word", cdi.board->cpu.currentPC, addr, data}); })
        return data;
    }

    if(addr == 0x4FFFE0) // word size: MSB is 0, LSB is the register
    {
        const uint16_t data = registerCSR2R;
        if(flags & Trigger)
            registerCSR2R = 0; // clear IT1, IT2 and BE bits on status read
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::VDSC, "Get", "Word", cdi.board->cpu.currentPC, addr, data}); })
        return data;
    }

    if(addr == 0x4FFFF0) // word size: MSB is 0, LSB is the register
    {
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::VDSC, "Get", "Word", cdi.board->cpu.currentPC, addr, registerCSR1R}); })
        return registerCSR1R;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Word", cdi.board->cpu.currentPC, addr, 0}); })
    return 0;
}

uint32_t MCD212::GetLong(const uint32_t addr, const uint8_t flags)
{
    return (uint32_t)GetWord(addr, flags) << 16 | GetWord(addr + 2, flags);
}

void MCD212::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x400000)
    {
        memory[addr] = data;
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::RAM, "Set", "Byte", cdi.board->cpu.currentPC, addr, data}); })
        return;
    }

    if(addr >= 0x4FFFE0 && addr < 0x500000)
    {
        if(isEven(addr))
        {
            internalRegisters[addr - 0x4FFFE0] &= 0x00FF;
            internalRegisters[addr - 0x4FFFE0] |= (uint16_t)data << 8;
        }
        else
        {
            internalRegisters[addr - 0x4FFFE0] &= 0xFF00;
            internalRegisters[addr - 0x4FFFE0] |= data;
        }
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::VDSC, "Set", "Byte", cdi.board->cpu.currentPC, addr, data}); })
        return;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Byte", cdi.board->cpu.currentPC, addr, data}); })
}

void MCD212::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x400000)
    {
        memory[addr]     = data >> 8;
        memory[addr + 1] = data;
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::RAM, "Set", "Word", cdi.board->cpu.currentPC, addr, data}); })
        return;
    }

    if(addr >= 0x4FFFE0 && addr < 0x500000)
    {
        internalRegisters[addr - 0x4FFFE0] = data;
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::VDSC, "Set", "Word", cdi.board->cpu.currentPC, addr, data}); })
        return;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Word", cdi.board->cpu.currentPC, addr, data}); })
}
