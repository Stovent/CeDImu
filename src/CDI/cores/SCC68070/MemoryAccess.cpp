#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

uint8_t SCC68070::GetByte(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const BusFlags flags)
{
    if(mode == 0)
        return D[reg] & 0x000000FF;

    if(mode == 1)
        return A(reg) & 0x000000FF;

    if(mode == 7 && reg == 4)
    {
        calcTime += ITIBW;
        return GetNextWord() & 0x00FF;
    }

    lastAddress = GetEffectiveAddress(mode, reg, 1, calcTime);
    return GetByte(lastAddress, flags);
}

uint16_t SCC68070::GetWord(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const BusFlags flags)
{
    if(mode == 0)
        return D[reg] & 0x0000FFFF;

    if(mode == 1)
        return A(reg) & 0x0000FFFF;

    if(mode == 7 && reg == 4)
    {
        calcTime += ITIBW;
        return GetNextWord();
    }

    lastAddress = GetEffectiveAddress(mode, reg, 2, calcTime);
    return GetWord(lastAddress, flags);
}

uint32_t SCC68070::GetLong(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const BusFlags flags)
{
    if(mode == 0)
        return D[reg];

    if(mode == 1)
        return A(reg);

    if(mode == 7 && reg == 4)
    {
        calcTime += ITIL;
        return GetNextWord() << 16 | GetNextWord();
    }

    lastAddress = GetEffectiveAddress(mode, reg, 4, calcTime);
    return GetLong(lastAddress, flags);
}

void SCC68070::SetByte(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t data, const BusFlags flags)
{
    if(mode == 0)
    {
        D[reg] &= 0xFFFFFF00;
        D[reg] |= data;
    }
    else
    {
        lastAddress = GetEffectiveAddress(mode, reg, 1, calcTime);
        SetByte(lastAddress, data, flags);
    }
}

void SCC68070::SetWord(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint16_t data, const BusFlags flags)
{
    if(mode == 0)
    {
        D[reg] &= 0xFFFF0000;
        D[reg] |= data;
    }
    else if(mode == 1)
    {
        A(reg) = signExtend<int16_t, uint32_t>(data);
    }
    else
    {
        lastAddress = GetEffectiveAddress(mode, reg, 2, calcTime);
        SetWord(lastAddress, data, flags);
    }
}

void SCC68070::SetLong(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint32_t data, const BusFlags flags)
{
    if(mode == 0)
    {
        D[reg] = data;
    }
    else if(mode == 1)
    {
        A(reg) = data;
    }
    else
    {
        lastAddress = GetEffectiveAddress(mode, reg, 4, calcTime);
        SetLong(lastAddress, data, flags);
    }
}

uint8_t SCC68070::GetByte(const uint32_t addr, const BusFlags flags)
{
    if(addr >= Peripheral::Base && addr < Peripheral::Last && GetS())
    {
        const uint8_t data = GetPeripheral(addr);
        LOG(if(cdi.m_callbacks.HasOnLogMemoryAccess()) \
                cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CPU, "Get", "Byte", currentPC, addr, data});)
        return data;
    }

    const uint8_t data = cdi.GetByte(addr, flags);
    return data;
}

uint16_t SCC68070::GetWord(const uint32_t addr, const BusFlags flags)
{
    if(!isEven(addr))
        throw Exception(AddressError);

    if(addr >= Peripheral::Base && addr < Peripheral::Last && GetS())
    {
        const uint16_t data = as<uint16_t>(GetPeripheral(addr)) << 8 | GetPeripheral(addr + 1);
        LOG(if(cdi.m_callbacks.HasOnLogMemoryAccess()) \
                cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CPU, "Get", "Word", currentPC, addr, data});)
        return data;
    }

    const uint16_t data = cdi.GetWord(addr, flags);
    return data;
}

uint32_t SCC68070::GetLong(const uint32_t addr, const BusFlags flags)
{
    return as<uint32_t>(GetWord(addr, flags)) << 16 | GetWord(addr + 2, flags);
}

void SCC68070::SetByte(const uint32_t addr, const uint8_t data, const BusFlags flags)
{
    if(addr >= Peripheral::Base && addr < Peripheral::Last && GetS())
    {
        SetPeripheral(addr, data);
        LOG(if(cdi.m_callbacks.HasOnLogMemoryAccess()) \
                cdi.m_callbacks.OnLogMemoryAccess({MemoryAccessLocation::CPU, "Set", "Byte", currentPC, addr, data});)
        return;
    }

    cdi.SetByte(addr, data, flags);
    return;
}

void SCC68070::SetWord(const uint32_t addr, const uint16_t data, const BusFlags flags)
{
    if(!isEven(addr))
        throw Exception(AddressError);

    if(addr >= Peripheral::Base && addr < Peripheral::Last && GetS())
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

void SCC68070::SetLong(const uint32_t addr, const uint32_t data, const BusFlags flags)
{
    SetWord(addr, data >> 16, flags);
    SetWord(addr + 2, data, flags);
}
