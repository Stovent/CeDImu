#include "SoftCDI.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../common/utils.hpp"

uint8_t SoftCDI::PeekByte(const uint32_t addr) const noexcept
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
    if(addr >= 0x320000 && addr < 0x324000 && isEven(addr))
    {
        return m_timekeeper->PeekByte((addr - 0x320000) >> 1);
    }

    // if(addr >= 0x4FFFE0 && addr < 0x500000)
    if(addr == 0x4FFFF1)
    {
        return m_csr1r;
    }

    if(addr >= SCC68070::Peripheral::Base && addr < SCC68070::Peripheral::Last)
    {
        return m_cpu.PeekPeripheral(addr - SCC68070::Peripheral::Base);
    }

    std::terminate(); // TODO: is this the best way? or should I return 0 or an optional?
}

uint16_t SoftCDI::PeekWord(const uint32_t addr) const noexcept
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

    if(addr >= SCC68070::Peripheral::Base && addr < SCC68070::Peripheral::Last)
    {
        return m_cpu.PeekPeripheral(addr - SCC68070::Peripheral::Base);
    }

    std::terminate(); // TODO: is this the best way? or should I return 0 or an optional?
}

uint32_t SoftCDI::PeekLong(const uint32_t addr) const noexcept
{
    return as<uint32_t>(PeekWord(addr)) << 16 | PeekWord(addr + 2);
}

uint8_t SoftCDI::GetByte(const uint32_t addr, const BusFlags flags)
{
    MemoryAccessLocation location;
    uint8_t data;

    if(addr < RAM0End)
    {
        data = m_ram0[addr];
        location = MemoryAccessLocation::RAM;
    }
    else if(addr >= RAM1Begin && addr < RAM1End)
    {
        data = m_ram1[addr - RAM1Begin];
        location = MemoryAccessLocation::RAM;
    }
    else if(addr >= BIOSBegin && addr < BIOSEnd)
    {
        data = m_bios[addr - BIOSBegin];
        location = MemoryAccessLocation::BIOS;
    }
    else if(addr >= SlaveBegin && addr < SlaveEnd && !isEven(addr))
    {
        // These are below the BIOS for performance reasons, it is useless to check for them on every memory read before the bios.
        data = 0; // dummy slave read.
        // data = m_slave->GetByte((addr - SlaveBegin) >> 1);
        location = MemoryAccessLocation::Slave;
    }
    else if(addr >= TimekeeperBegin && addr < TimekeeperEnd && isEven(addr))
    {
        data = m_timekeeper->GetByte((addr - TimekeeperBegin) >> 1, flags);
        location = MemoryAccessLocation::RTC;
    }
    else if(addr == 0x4FFFF1)
    // if(addr >= MCD212RegistersBegin && addr < MCD212RegistersEnd)
    {
        // TODO: implement video reg.
        m_csr1r = ~m_csr1r;
        data = m_csr1r;
        location = MemoryAccessLocation::VDSC;
    }
    else
    {
        LOG_FLAGS(flags, OutOfRange, "Get", "Byte", m_cpu.currentPC, addr, 0);
        throw SCC68070::Exception(SCC68070::BusError);
    }

    LOG(if(flags.log && m_callbacks.HasOnLogMemoryAccess()) \
        m_callbacks.OnLogMemoryAccess({location, "Get", "Byte", m_cpu.currentPC, addr, data});)

    return data;
}

uint16_t SoftCDI::GetWord(const uint32_t addr, const BusFlags flags)
{
    MemoryAccessLocation location;
    uint16_t data;

    if(addr < RAM0End)
    {
        data = GET_ARRAY16(m_ram0, addr);
        location = MemoryAccessLocation::RAM;
    }
    else if(addr >= RAM1Begin && addr < RAM1End)
    {
        data = GET_ARRAY16(m_ram1, addr - RAM1Begin);
        location = MemoryAccessLocation::RAM;
    }
    else if(addr >= BIOSBegin && addr < BIOSEnd)
    {
        data = GET_ARRAY16(m_bios, addr - BIOSBegin);
        location = MemoryAccessLocation::BIOS;
    }
    else
    {
        LOG_FLAGS(flags, OutOfRange, "Get", "Word", m_cpu.currentPC, addr, 0);
        throw SCC68070::Exception(SCC68070::BusError);
    }

    LOG(if(flags.log && m_callbacks.HasOnLogMemoryAccess()) \
        m_callbacks.OnLogMemoryAccess({location, "Get", "Word", m_cpu.currentPC, addr, data});)

    return data;
}

uint32_t SoftCDI::GetLong(const uint32_t addr, const BusFlags flags)
{
    return as<uint32_t>(GetWord(addr, flags)) << 16 | GetWord(addr + 2, flags);
}

void SoftCDI::SetByte(const uint32_t addr, const uint8_t data, const BusFlags flags)
{
    MemoryAccessLocation location;

    // Order is to optimise access with the more often checkd first.
    if(addr < RAM0End)
    {
        m_ram0[addr] = data;
        location = MemoryAccessLocation::RAM;
    }
    else if(addr >= RAM1Begin && addr < RAM1End)
    {
        m_ram1[addr - RAM1Begin] = data;
        location = MemoryAccessLocation::RAM;
    }
    else if(addr >= TimekeeperBegin && addr < TimekeeperEnd && isEven(addr))
    {
        m_timekeeper->SetByte((addr - TimekeeperBegin) >> 1, data, flags);
        location = MemoryAccessLocation::RTC;
    }
    else if(addr >= SlaveBegin && addr < SlaveEnd)
    {
        // Ignore slave memory writes.
        location = MemoryAccessLocation::Slave;
    }
    else
    {
        LOG_FLAGS(flags, OutOfRange, "Set", "Byte", m_cpu.currentPC, addr, data);
        throw SCC68070::Exception(SCC68070::BusError);
    }

    LOG(if(flags.log && m_callbacks.HasOnLogMemoryAccess()) \
        m_callbacks.OnLogMemoryAccess({location, "Set", "Byte", m_cpu.currentPC, addr, data});)
}

void SoftCDI::SetWord(const uint32_t addr, const uint16_t data, const BusFlags flags)
{
    MemoryAccessLocation location;

    if(addr < RAM0End)
    {
        m_ram0[addr] = data >> 8;
        m_ram0[addr + 1] = data;
        location = MemoryAccessLocation::RAM;
    }
    else if(addr >= RAM1Begin && addr < RAM1End)
    {
        m_ram1[addr - RAM1Begin] = data >> 8;
        m_ram1[addr - RAM1Begin + 1] = data;
        location = MemoryAccessLocation::RAM;
    }
    else if(addr >= MCD212RegistersBegin && addr < MCD212RegistersEnd)
    {
        // Ignore MCD212 writes.
        location = MemoryAccessLocation::VDSC;
    }
    else
    {
        LOG_FLAGS(flags, OutOfRange, "Set", "Word", m_cpu.currentPC, addr, data);
        throw SCC68070::Exception(SCC68070::BusError);
    }

    LOG(if(flags.log && m_callbacks.HasOnLogMemoryAccess()) \
        m_callbacks.OnLogMemoryAccess({location, "Set", "Word", m_cpu.currentPC, addr, data});)
}

void SoftCDI::SetLong(const uint32_t addr, const uint32_t data, const BusFlags flags)
{
    SetWord(addr, data >> 16, flags);
    SetWord(addr + 2, data, flags);
}
