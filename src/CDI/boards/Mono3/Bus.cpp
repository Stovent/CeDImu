#include "Mono3.hpp"
#include "../../CDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../common/utils.hpp"

uint8_t Mono3::PeekByte(const uint32_t addr) const noexcept
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        return m_mcd212.PeekByte(addr);
    }

    if(addr >= 0x300000 && addr < 0x304000)
    {
        const uint16_t data = m_ciap.PeekWord(addr - 0x300000);
        return isEven(addr) ? data >> 8 : data;
    }

    if(addr >= 0x310000 && addr < 0x31001E && !isEven(addr))
    {
        return m_slave->PeekByte((addr - 0x310000) >> 1);
    }

    if(addr >= 0x320000 && addr < m_nvramMaxAddress && isEven(addr))
    {
        return m_timekeeper->PeekByte((addr - 0x320000) >> 1);
    }

    if(addr >= SCC68070::Peripheral::Base && addr < SCC68070::Peripheral::Last)
    {
        return m_cpu.PeekPeripheral(addr - SCC68070::Peripheral::Base);
    }

    std::terminate(); // TODO: is this the best way? or should I return 0 or an optional?
}

uint16_t Mono3::PeekWord(const uint32_t addr) const noexcept
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        return m_mcd212.PeekWord(addr);
    }

    if(addr >= 0x300000 && addr < 0x304000)
    {
        return m_ciap.PeekWord(addr - 0x300000);
    }

    if(addr >= 0x310000 && addr < 0x31001E)
    {
        return m_slave->PeekByte((addr - 0x310000) >> 1);
    }

    if(addr >= 0x320000 && addr < m_nvramMaxAddress)
    {
        return as<uint16_t>(m_timekeeper->PeekByte((addr - 0x320000) >> 1)) << 8;
    }

    if(addr >= SCC68070::Peripheral::Base && addr < SCC68070::Peripheral::Last)
    {
        return m_cpu.PeekPeripheral(addr - SCC68070::Peripheral::Base);
    }

    std::terminate(); // TODO: is this the best way? or should I return 0 or an optional?
}

uint32_t Mono3::PeekLong(const uint32_t addr) const noexcept
{
    return as<uint32_t>(PeekWord(addr)) << 16 | PeekWord(addr + 2);
}

uint8_t Mono3::GetByte(const uint32_t addr, const BusFlags flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        return m_mcd212.GetByte(addr, flags);
    }

    if(addr >= 0x300000 && addr < 0x304000)
    {
        const uint16_t data = m_ciap.GetWord(addr - 0x300000, flags);
        return isEven(addr) ? data >> 8 : data;
    }

    if(addr >= 0x310000 && addr < 0x31001E && !isEven(addr))
    {
        return m_slave->GetByte((addr - 0x310000) >> 1, flags);
    }

    if(addr >= 0x320000 && addr < m_nvramMaxAddress && isEven(addr))
    {
        return m_timekeeper->GetByte((addr - 0x320000) >> 1, flags);
    }

    LOG(if(flags.log && m_callbacks.HasOnLogMemoryAccess()) \
            m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Byte", m_cpu.currentPC, addr, 0});)
    throw SCC68070::Exception(SCC68070::BusError);
}

uint16_t Mono3::GetWord(const uint32_t addr, const BusFlags flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        return m_mcd212.GetWord(addr, flags);
    }

    if(addr >= 0x300000 && addr < 0x304000)
    {
        return m_ciap.GetWord(addr - 0x300000, flags);
    }

    if(addr >= 0x310000 && addr < 0x31001E)
    {
        return m_slave->GetByte((addr - 0x310000) >> 1, flags);
    }

    if(addr >= 0x320000 && addr < m_nvramMaxAddress)
    {
        return as<uint16_t>(m_timekeeper->GetByte((addr - 0x320000) >> 1, flags)) << 8;
    }

    LOG(if(flags.log && m_callbacks.HasOnLogMemoryAccess()) \
            m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Get", "Word", m_cpu.currentPC, addr, 0});)
    throw SCC68070::Exception(SCC68070::BusError);
}

uint32_t Mono3::GetLong(const uint32_t addr, const BusFlags flags)
{
    return as<uint32_t>(GetWord(addr, flags)) << 16 | GetWord(addr + 2, flags);
}

void Mono3::SetByte(const uint32_t addr, const uint8_t data, const BusFlags flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x4FFFE0 && addr < 0x500000))
    {
        m_mcd212.SetByte(addr, data, flags);
        return;
    }

    if(addr >= 0x300000 && addr < 0x304000)
    {
        const uint16_t word = m_ciap.GetWord(addr - 0x300000, flags);
        if(isEven(addr))
            m_ciap.SetWord(addr - 0x300000, (word & 0x00FF) | as<uint16_t>(data) << 8, flags);
        else
            m_ciap.SetWord(addr - 0x300000, (word & 0xFF00) | data, flags);
        return;
    }

    if(addr >= 0x310000 && addr < 0x31001E && !isEven(addr))
    {
        m_slave->SetByte((addr - 0x310000) >> 1, data, flags);
        return;
    }

    if(addr >= 0x320000 && addr < m_nvramMaxAddress && isEven(addr))
    {
        m_timekeeper->SetByte((addr - 0x320000) >> 1, data, flags);
        return;
    }

    LOG(if(flags.log && m_callbacks.HasOnLogMemoryAccess()) \
            m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Byte", m_cpu.currentPC, addr, data});)
    throw SCC68070::Exception(SCC68070::BusError);
}

void Mono3::SetWord(const uint32_t addr, const uint16_t data, const BusFlags flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x4FFFE0 && addr < 0x500000))
    {
        m_mcd212.SetWord(addr, data, flags);
        return;
    }

    if(addr >= 0x300000 && addr < 0x304000)
    {
        return m_ciap.SetWord(addr - 0x300000, data, flags);
    }

    if(addr >= 0x310000 && addr < 0x31001E)
    {
        m_slave->SetByte((addr - 0x310000) >> 1, data, flags);
        return;
    }

    if(addr >= 0x320000 && addr < m_nvramMaxAddress)
    {
        m_timekeeper->SetByte((addr - 0x320000) >> 1, data >> 8, flags);
        return;
    }

    LOG(if(flags.log && m_callbacks.HasOnLogMemoryAccess()) \
            m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::OutOfRange, "Set", "Word", m_cpu.currentPC, addr, data});)
    throw SCC68070::Exception(SCC68070::BusError);
}

void Mono3::SetLong(const uint32_t addr, const uint32_t data, const BusFlags flags)
{
    SetWord(addr, data >> 16, flags);
    SetWord(addr + 2, data, flags);
}
