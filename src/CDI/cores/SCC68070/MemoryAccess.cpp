#include "SCC68070.hpp"
#include "../../boards/Board.hpp"
#include "../../common/utils.hpp"

uint8_t SCC68070::GetByte(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t flags)
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

uint16_t SCC68070::GetWord(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t flags)
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

uint32_t SCC68070::GetLong(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t flags)
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

void SCC68070::SetByte(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint8_t data, const uint8_t flags)
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

void SCC68070::SetWord(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint16_t data, const uint8_t flags)
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

void SCC68070::SetLong(const uint8_t mode, const uint8_t reg, uint16_t& calcTime, const uint32_t data, const uint8_t flags)
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

uint8_t SCC68070::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x80000000 || addr >= 0xC0000000)
    {
        const uint8_t data = board.GetByte(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet byte at 0x%X: %d %d 0x%X\n", currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    if(addr >= SCC68070Peripherals::Base && addr < SCC68070Peripherals::Last)
    {
        if(GetS())
        {
            const uint8_t data = GetPeripheral(addr);
            LOG(if(flags & Log) { fprintf(out, "%X\tGet peripheral at 0x%X: %d %d 0x%X\n", currentPC, addr, (int8_t)data, data, data); })
            return data;
        }

        const uint8_t data = board.GetByte(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet byte at 0x%X: %d %d 0x%X\n", currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tGet byte OUT OF RANGE at 0x%X\n", currentPC, addr); })
    throw SCC68070Exception(BusError, 0);
}

uint16_t SCC68070::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(!isEven(addr))
        throw SCC68070Exception(AddressError, 0);

    if(addr < 0x80000000 || addr >= 0xC0000000)
    {
        const uint16_t data = board.GetWord(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet word at 0x%X: %d %d 0x%X\n", currentPC, addr, (int16_t)data, data, data); })
        return data;
    }

    if(addr >= SCC68070Peripherals::Base && addr < SCC68070Peripherals::Last)
    {
        if(GetS())
        {
            const uint16_t data = (uint16_t)GetPeripheral(addr) << 8 | GetPeripheral(addr + 1);
            LOG(if(flags & Log) { fprintf(out, "%X\tGet peripheral at 0x%X: %d %d 0x%X\n", currentPC, addr, (int16_t)data, data, data); })
            return data;
        }

        const uint16_t data = board.GetWord(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet word at 0x%X: %d %d 0x%X\n", currentPC, addr, (int16_t)data, data, data); })
        return data;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tGet word OUT OF RANGE at 0x%X\n", currentPC, addr); })
    throw SCC68070Exception(BusError, 0);
}

uint32_t SCC68070::GetLong(const uint32_t addr, const uint8_t flags)
{
    if(!isEven(addr))
        throw SCC68070Exception(AddressError, 0);

    if(addr < 0x80000000 || addr >= 0xC0000000)
    {
        const uint32_t data = board.GetLong(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet long at 0x%X: %d %d 0x%X\n", currentPC, addr, (int32_t)data, data, data); })
        return data;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tGet long OUT OF RANGE at 0x%X\n", currentPC, addr); })
    throw SCC68070Exception(BusError, 0);
}

void SCC68070::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x80000000 || addr >= 0xC0000000)
    {
        board.SetByte(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet byte at 0x%X: %d %d 0x%X\n", currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    if(addr >= SCC68070Peripherals::Base && addr < SCC68070Peripherals::Last)
    {
        if(GetS())
        {
            SetPeripheral(addr, data);
            LOG(if(flags & Log) { fprintf(out, "%X\tSet peripheral at 0x%X: %d %d 0x%X\n", currentPC, addr, (int8_t)data, data, data); })
            return;
        }

        board.SetByte(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet byte at 0x%X: %d %d 0x%X\n", currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tSet byte OUT OF RANGE at 0x%X: %d %d 0x%X\n", currentPC, addr, (int8_t)data, data, data); })
    throw SCC68070Exception(BusError, 0);
}

void SCC68070::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(!isEven(addr))
        throw SCC68070Exception(AddressError, 0);

    if(addr < 0x80000000 || addr >= 0xC0000000)
    {
        board.SetWord(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet word at 0x%X: %d %d 0x%X\n", currentPC, addr, (int16_t)data, data, data); })
        return;
    }

    if(addr >= SCC68070Peripherals::Base && addr < SCC68070Peripherals::Last)
    {
        if(GetS())
        {
            SetPeripheral(addr, data >> 8);
            SetPeripheral(addr + 1, data);
            LOG(if(flags & Log) { fprintf(out, "%X\tSet peripheral at 0x%X: %d %d 0x%X\n", currentPC, addr, (int16_t)data, data, data); })
            return;
        }

        board.SetWord(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet word at 0x%X: %d %d 0x%X\n", currentPC, addr, (int16_t)data, data, data); })
        return;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tSet word OUT OF RANGE at 0x%X: %d %d 0x%X\n", currentPC, addr, (int16_t)data, data, data); })
    throw SCC68070Exception(BusError, 0);
}

void SCC68070::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    if(!isEven(addr))
        throw SCC68070Exception(AddressError, 0);

    if(addr < 0x80000000 || addr >= 0xC0000000)
    {
        board.SetLong(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet long at 0x%X: %d %d 0x%X\n", currentPC, addr, (int32_t)data, data, data); })
        return;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tSet long OUT OF RANGE at 0x%X: %d %d 0x%X\n", currentPC, addr, (int32_t)data, data, data); })
    throw SCC68070Exception(BusError, 0);
}
