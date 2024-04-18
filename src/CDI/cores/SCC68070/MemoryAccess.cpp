#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

uint8_t SCC68070::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr >= Peripheral::Base && addr < Peripheral::Last && m68000Registers->sr.s)
    {
        const uint8_t data = GetPeripheral(addr);
        LOG(if(cdi.m_callbacks.HasOnLogMemoryAccess()) \
                cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CPU, "Get", "Byte", currentPC, addr, data});)
        return data;
    }

    const uint8_t data = cdi.GetByte(addr, flags);
    return data;
}

uint16_t SCC68070::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr >= Peripheral::Base && addr < Peripheral::Last && m68000Registers->sr.s)
    {
        const uint16_t data = (uint16_t)GetPeripheral(addr) << 8 | GetPeripheral(addr + 1);
        LOG(if(cdi.m_callbacks.HasOnLogMemoryAccess()) \
                cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CPU, "Get", "Word", currentPC, addr, data});)
        return data;
    }

    const uint16_t data = cdi.GetWord(addr, flags);
    return data;
}

uint32_t SCC68070::GetLong(const uint32_t addr, const uint8_t flags)
{
    return (uint32_t)GetWord(addr, flags) << 16 | GetWord(addr + 2, flags);
}

void SCC68070::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr >= Peripheral::Base && addr < Peripheral::Last && m68000Registers->sr.s)
    {
        SetPeripheral(addr, data);
        LOG(if(cdi.m_callbacks.HasOnLogMemoryAccess()) \
                cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CPU, "Set", "Byte", currentPC, addr, data});)
        return;
    }

    cdi.SetByte(addr, data, flags);
    return;
}

void SCC68070::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr >= Peripheral::Base && addr < Peripheral::Last && m68000Registers->sr.s)
    {
        SetPeripheral(addr, data >> 8);
        SetPeripheral(addr + 1, data);
        LOG(if(cdi.m_callbacks.HasOnLogMemoryAccess()) \
                cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CPU, "Set", "Word", currentPC, addr, data});)
        return;
    }

    cdi.SetWord(addr, data, flags);
    return;
}

void SCC68070::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    SetWord(addr, data >> 16, flags);
    SetWord(addr + 2, data, flags);
}
