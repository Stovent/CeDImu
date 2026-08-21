#include "SCC68070-m68000.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

uint8_t SCC68070::GetByte(const uint32_t addr, const BusFlags flags)
{
    if(addr >= Peripheral::Base && addr < Peripheral::Last && m_regs->sr.s)
    {
        const uint8_t data = GetPeripheral(addr, flags);
        LOG(if(m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
                m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CPU, "Get", "Byte", currentPC, addr, data});)
        return data;
    }

    const uint8_t data = m_cdi.GetByte(addr, flags);
    return data;
}

uint16_t SCC68070::GetWord(const uint32_t addr, const BusFlags flags)
{
    if(!isEven(addr))
        throw Exception(AddressError);

    if(addr >= Peripheral::Base && addr < Peripheral::Last && m_regs->sr.s)
    {
        const uint16_t data = as<uint16_t>(GetPeripheral(addr, flags)) << 8 | GetPeripheral(addr + 1, flags);
        LOG(if(m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
                m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CPU, "Get", "Word", currentPC, addr, data});)
        return data;
    }

    const uint16_t data = m_cdi.GetWord(addr, flags);
    return data;
}

uint32_t SCC68070::GetLong(const uint32_t addr, const BusFlags flags)
{
    return as<uint32_t>(GetWord(addr, flags)) << 16 | GetWord(addr + 2, flags);
}

void SCC68070::SetByte(const uint32_t addr, const uint8_t data, const BusFlags flags)
{
    if(addr >= Peripheral::Base && addr < Peripheral::Last && m_regs->sr.s)
    {
        SetPeripheral(addr, data, flags);
        LOG(if(m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
                m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CPU, "Set", "Byte", currentPC, addr, data});)
        return;
    }

    m_cdi.SetByte(addr, data, flags);
}

void SCC68070::SetWord(const uint32_t addr, const uint16_t data, const BusFlags flags)
{
    if(!isEven(addr))
        throw Exception(AddressError);

    if(addr >= Peripheral::Base && addr < Peripheral::Last && m_regs->sr.s)
    {
        SetPeripheral(addr, data >> 8, flags);
        SetPeripheral(addr + 1, data, flags);
        LOG(if(m_cdi.m_callbacks.HasOnLogMemoryAccess()) \
                m_cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CPU, "Set", "Word", currentPC, addr, data});)
        return;
    }

    m_cdi.SetWord(addr, data, flags);
}

void SCC68070::SetLong(const uint32_t addr, const uint32_t data, const BusFlags flags)
{
    SetWord(addr, data >> 16, flags);
    SetWord(addr + 2, data, flags);
}
