#include "MiniMMC.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../common/utils.hpp"

uint8_t MiniMMC::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        return masterVDSC.GetByte(addr, flags);
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        return slaveVDSC.GetByte(addr, flags);
    }

//    if(addr >= 0x200000 && addr < 0x200008 && !isEven(addr))
//    {
//        return 0; // TODO: slave->GetByte((addr - 0x200000) >> 1);
//    }

    if(addr >= 0x3F8000 && addr < 0x3FC000 && isEven(addr))
    {
        return timekeeper->GetByte((addr - 0x3F8000) >> 1);

    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Byte", cpu.currentPC, addr, 0}); })
    throw SCC68070::Exception(SCC68070::BusError, 0);
}

uint16_t MiniMMC::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        return masterVDSC.GetWord(addr, flags);
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        return slaveVDSC.GetWord(addr, flags);
    }

//    if(addr >= 0x200000 && addr < 0x200008)
//    {
//        return 0; // TODO: slave->GetByte((addr - 0x200000) >> 1);
//    }

    if(addr >= 0x3F8000 && addr < 0x3FC000)
    {
        return (uint16_t)timekeeper->GetByte((addr - 0x3F8000) >> 1) << 8;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Word", cpu.currentPC, addr, 0}); })
    throw SCC68070::Exception(SCC68070::BusError, 0);
}

uint32_t MiniMMC::GetLong(const uint32_t addr, const uint8_t flags)
{
    return (uint32_t)GetWord(addr, flags) << 16 | GetWord(addr + 2, flags);
}

void MiniMMC::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        masterVDSC.SetByte(addr, data, flags);
        return;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        slaveVDSC.SetByte(addr, data, flags);
        return;
    }

//    if(addr >= 0x200000 && addr < 0x200008 && !isEven(addr))
//    {
//        // slave->SetByte((addr - 0x200000) >> 1, data);
//        return;
//    }

    if(addr >= 0x3F8000 && addr < 0x3FC000 && isEven(addr))
    {
        timekeeper->SetByte((addr - 0x3F8000) >> 1, data);
        return;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Byte", cpu.currentPC, addr, data}); })
    throw SCC68070::Exception(SCC68070::BusError, 0);
}

void MiniMMC::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        masterVDSC.SetWord(addr, data, flags);
        return;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        slaveVDSC.SetWord(addr, data, flags);
        return;
    }

//    if(addr >= 0x200000 && addr < 0x200008)
//    {
//        // slave->SetByte((addr - 0x200000) >> 1, d);
//        return;
//    }

    if(addr >= 0x3F8000 && addr < 0x3FC000)
    {
        const uint8_t d = data >> 8;
        timekeeper->SetByte((addr - 0x3F8000) >> 1, d);
        return;
    }

    LOG(if(flags & Log) { if(cdi.callbacks.HasOnLogMemoryAccess()) \
            cdi.callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Word", cpu.currentPC, addr, data}); })
    throw SCC68070::Exception(SCC68070::BusError, 0);
}

void MiniMMC::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    SetWord(addr, data >> 16, flags);
    SetWord(addr + 2, data, flags);
}
