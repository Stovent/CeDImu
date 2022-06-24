#include "Mono2.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../common/utils.hpp"

uint8_t Mono2::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        return mcd212.GetByte(addr, flags);
    }

    if(addr >= 0x310000 && addr < 0x31001E && !isEven(addr))
    {
        return slave->GetByte((addr - 0x310000) >> 1);
    }

    if(addr >= 0x320000 && addr < nvramMaxAddress && isEven(addr))
    {
        return timekeeper->GetByte((addr - 0x320000) >> 1);
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Byte", cpu.currentPC, addr, 0}); })
    throw SCC68070::Exception(Vector::AccessError);
}

uint16_t Mono2::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        return mcd212.GetWord(addr, flags);
    }

    if(addr >= 0x310000 && addr < 0x31001E)
    {
        return slave->GetByte((addr - 0x310000) >> 1);
    }

    if(addr >= 0x320000 && addr < nvramMaxAddress)
    {
        return (uint16_t)timekeeper->GetByte((addr - 0x320000) >> 1) << 8;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Word", cpu.currentPC, addr, 0}); })
    throw SCC68070::Exception(Vector::AccessError);
}

uint32_t Mono2::GetLong(const uint32_t addr, const uint8_t flags)
{
    return (uint32_t)GetWord(addr, flags) << 16 | GetWord(addr + 2, flags);
}

void Mono2::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x4FFFE0 && addr < 0x500000))
    {
        mcd212.SetByte(addr, data, flags);
        return;
    }

    if(addr >= 0x310000 && addr < 0x31001E && !isEven(addr))
    {
        slave->SetByte((addr - 0x310000) >> 1, data);
        return;
    }

    if(addr >= 0x320000 && addr < nvramMaxAddress && isEven(addr))
    {
        timekeeper->SetByte((addr - 0x320000) >> 1, data);
        return;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Byte", cpu.currentPC, addr, data}); })
    throw SCC68070::Exception(Vector::AccessError);
}

void Mono2::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x4FFFE0 && addr < 0x500000))
    {
        mcd212.SetWord(addr, data, flags);
        return;
    }

    if(addr >= 0x310000 && addr < 0x31001E)
    {
        slave->SetByte((addr - 0x310000) >> 1, data);
        return;
    }

    if(addr >= 0x320000 && addr < nvramMaxAddress)
    {
        timekeeper->SetByte((addr - 0x320000) >> 1, data >> 8);
        return;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Word", cpu.currentPC, addr, data}); })
    throw SCC68070::Exception(Vector::AccessError);
}

void Mono2::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    SetWord(addr, data >> 16, flags);
    SetWord(addr + 2, data, flags);
}
