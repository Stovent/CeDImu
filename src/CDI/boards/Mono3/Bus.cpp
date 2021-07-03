#include "Mono3.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../common/utils.hpp"

uint8_t Mono3::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        return mcd212.GetByte(addr, flags);
    }

    if(addr >= 0x300000 && addr < 0x310000) // TODO
    {
        return 0;
    }

    if(addr >= 0x310000 && addr < 0x31001E && !isEven(addr))
    {
        return slave->GetByte((addr - 0x310000) >> 1);
    }

    if(addr >= 0x320000 && addr < 0x324000 && isEven(addr))
    {
        const uint8_t data = timekeeper.GetByte((addr - 0x320000) >> 1);
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({"RTC", "Get", "Byte", cpu.currentPC, addr, data}); })
        return data;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({"OUT OF RANGE", "Get", "Byte", cpu.currentPC, addr, 0}); })
    throw SCC68070Exception(BusError, 0);
}

uint16_t Mono3::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        return mcd212.GetWord(addr, flags);
    }

    if(addr >= 0x300000 && addr < 0x310000) // TODO
    {
        return 0; // addr == 0x3025C4 ? 0xCD02 : 0;
    }

    if(addr >= 0x310000 && addr < 0x31001E)
    {
        return slave->GetByte((addr - 0x310000) >> 1);
    }

    if(addr >= 0x320000 && addr < 0x324000)
    {
        const uint8_t data = timekeeper.GetByte((addr - 0x320000) >> 1);
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({"RTC", "Get", "Word", cpu.currentPC, addr, data}); })
        return (uint16_t)data << 8;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({"OUT OF RANGE", "Get", "Word", cpu.currentPC, addr, 0}); })
    throw SCC68070Exception(BusError, 0);
}

uint32_t Mono3::GetLong(const uint32_t addr, const uint8_t flags)
{
    return (uint32_t)GetWord(addr, flags) << 16 | GetWord(addr + 2, flags);
}

void Mono3::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x4FFFE0 && addr < 0x500000))
    {
        mcd212.SetByte(addr, data, flags);
        return;
    }

    if(addr >= 0x300000 && addr < 0x310000) // TODO
    {
        return;
    }

    if(addr >= 0x310000 && addr < 0x31001E && !isEven(addr))
    {
        slave->SetByte((addr - 0x310000) >> 1, data);
        return;
    }

    if(addr >= 0x320000 && addr < 0x324000 && isEven(addr))
    {
        timekeeper.SetByte((addr - 0x320000) >> 1, data);
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({"RTC", "Set", "Byte", cpu.currentPC, addr, data}); })
        return;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({"OUT OF RANGE", "Set", "Byte", cpu.currentPC, addr, data}); })
    throw SCC68070Exception(BusError, 0);
}

void Mono3::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x4FFFE0 && addr < 0x500000))
    {
        mcd212.SetWord(addr, data, flags);
        return;
    }

    if(addr >= 0x300000 && addr < 0x310000) // TODO
    {
        return;
    }

    if(addr >= 0x310000 && addr < 0x31001E)
    {
        slave->SetByte((addr - 0x310000) >> 1, data);
        return;
    }

    if(addr >= 0x320000 && addr < 0x324000)
    {
        const uint8_t d = data >> 8;
        timekeeper.SetByte((addr - 0x320000) >> 1, d);
        LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
                cdi.callbacks.OnLogMemoryAccess({"RTC", "Set", "Word", cpu.currentPC, addr, d}); })
        return;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({"OUT OF RANGE", "Set", "Word", cpu.currentPC, addr, data}); })
    throw SCC68070Exception(BusError, 0);
}

void Mono3::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    SetWord(addr, data >> 16, flags);
    SetWord(addr + 2, data, flags);
}
