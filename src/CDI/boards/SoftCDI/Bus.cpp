#include "SoftCDI.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../common/utils.hpp"

uint8_t SoftCDI::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000)
    {
        return m_ram0[addr];
    }

    if(addr >= 0x200000 && addr < 0x280000)
    {
        return m_ram1[addr - 0x200000];
    }

    if(addr >= 0x400000 && addr < 0x4FFFE0)
    {
        return m_bios[addr - 0x400000];
    }

    // These are below the BIOS for performance reasons, it is useless to check for them on every memory read before the bios.
    if(addr >= 0x310000 && addr < 0x31001E && !isEven(addr))
    {
        return 0; // dummy slave read.
        // return m_slave->GetByte((addr - 0x310000) >> 1);
    }

    if(addr >= 0x320000 && addr < 0x324000 && isEven(addr))
    {
        return m_timekeeper->GetByte((addr - 0x320000) >> 1);
    }

    // if(addr >= 0x4FFFE0 && addr < 0x500000)
    if(addr == 0x4FFFF1)
    {
        m_csr1r = ~m_csr1r;
        return m_csr1r;
    }

    LOG(if(flags & Log) { if(m_callbacks.HasOnLogMemoryAccess()) \
            m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Byte", m_cpu.currentPC, addr, 0}); })
    throw SCC68070::Exception(SCC68070::BusError, 0);
}

uint16_t SoftCDI::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000)
    {
        return GET_ARRAY16(m_ram0, addr);
    }

    if(addr >= 0x200000 && addr < 0x280000)
    {
        return GET_ARRAY16(m_ram1, addr - 0x200000);
    }

    if(addr >= 0x400000 && addr < 0x4FFFE0)
    {
        return GET_ARRAY16(m_bios, addr - 0x400000);
    }

    LOG(if(flags & Log) { if(m_callbacks.HasOnLogMemoryAccess()) \
            m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Word", m_cpu.currentPC, addr, 0}); })
    throw SCC68070::Exception(SCC68070::BusError, 0);
}

uint32_t SoftCDI::GetLong(const uint32_t addr, const uint8_t flags)
{
    return as<uint32_t>(GetWord(addr, flags)) << 16 | GetWord(addr + 2, flags);
}

void SoftCDI::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    // Order is to optimise access with the more often checkd first.
    if(addr < 0x080000)
    {
        m_ram0[addr] = data;
        return;
    }

    if(addr >= 0x200000 && addr < 0x280000)
    {
        m_ram1[addr - 0x200000] = data;
        return;
    }

    if(addr >= 0x320000 && addr < 0x324000 && isEven(addr))
    {
        m_timekeeper->SetByte((addr - 0x320000) >> 1, data);
        return;
    }

    if(addr >= 0x310000 && addr < 0x31001E)
        return; // Ignore slave memory writes.

    LOG(if(flags & Log) { if(m_callbacks.HasOnLogMemoryAccess()) \
            m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Byte", m_cpu.currentPC, addr, data}); })
    throw SCC68070::Exception(SCC68070::BusError, 0);
}

void SoftCDI::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x080000)
    {
        m_ram0[addr] = data >> 8;
        m_ram0[addr + 1] = data;
        return;
    }

    if(addr >= 0x200000 && addr < 0x280000)
    {
        m_ram1[addr - 0x200000] = data >> 8;
        m_ram1[addr - 0x1FFFFF] = data;
        return;
    }

    if(addr >= 0x4FFFE0 && addr < 0x500000)
    {
        return; // Ignore MCD212 writes.
    }

    LOG(if(flags & Log) { if(m_callbacks.HasOnLogMemoryAccess()) \
            m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Word", m_cpu.currentPC, addr, data}); })
    throw SCC68070::Exception(SCC68070::BusError, 0);
}

void SoftCDI::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    SetWord(addr, data >> 16, flags);
    SetWord(addr + 2, data, flags);
}
