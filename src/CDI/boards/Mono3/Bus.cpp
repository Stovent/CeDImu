#include "Mono3.hpp"
#include "../../common/utils.hpp"

uint8_t Mono3::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        const uint8_t data = mcd212.GetByte(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet byte in VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    if(addr >= 0x300000 && addr < 0x310000) // TODO
    {
        const uint8_t data = 0;
        LOG(if(flags & Log) { fprintf(out, "%X\tGet byte in CIAP at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    if(addr >= 0x310000 && addr < 0x320000) // TODO
    {
        const uint8_t data = 0;
        LOG(if(flags & Log) { fprintf(out, "%X\tGet byte in IKAT at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    if(addr >= 0x320000 && addr < 0x324000 && isEven(addr))
    {
        const uint8_t data = timekeeper.GetByte((addr - 0x320000) >> 1);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet byte in timekeeper at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tGet byte OUT OF RANGE at 0x%X\n", cpu.currentPC, addr); })
    throw SCC68070Exception(BusError, 0);
}

uint16_t Mono3::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        const uint16_t data = mcd212.GetWord(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet word in VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return data;
    }

    if(addr >= 0x300000 && addr < 0x310000) // TODO
    {
        const uint16_t data = 0;
        LOG(if(flags & Log) { fprintf(out, "%X\tGet word in CIAP at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return data;
    }

    if(addr >= 0x310000 && addr < 0x320000) // TODO
    {
        const uint16_t data = 0;
        LOG(if(flags & Log) { fprintf(out, "%X\tGet word in IKAT at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return data;
    }

    if(addr >= 0x320000 && addr < 0x324000)
    {
        const uint8_t data = timekeeper.GetByte((addr - 0x320000) >> 1);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet word in timekeeper at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return (uint16_t)data << 8;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tGet word OUT OF RANGE at 0x%X\n", cpu.currentPC, addr); })
    throw SCC68070Exception(BusError, 0);
}

uint32_t Mono3::GetLong(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        const uint32_t data = mcd212.GetLong(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet long in VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int32_t)data, data, data); })
        return data;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tGet long OUT OF RANGE at 0x%X\n", cpu.currentPC, addr); })
    throw SCC68070Exception(BusError, 0);
}

void Mono3::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        mcd212.SetByte(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet byte in VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    if(addr >= 0x300000 && addr < 0x310000) // TODO
    {
        LOG(if(flags & Log) { fprintf(out, "%X\tSet byte in CIAP at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    if(addr >= 0x310000 && addr < 0x320000) // TODO
    {
        LOG(if(flags & Log) { fprintf(out, "%X\tSet byte in IKAT at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    if(addr >= 0x320000 && addr < 0x324000 && isEven(addr))
    {
        timekeeper.SetByte((addr - 0x320000) >> 1, data);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet byte in timekeeper at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tSet byte OUT OF RANGE at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
    throw SCC68070Exception(BusError, 0);
}

void Mono3::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        mcd212.SetWord(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet word in VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return;
    }

    if(addr >= 0x300000 && addr < 0x310000) // TODO
    {
        LOG(if(flags & Log) { fprintf(out, "%X\tSet word in CIAP at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return;
    }

    if(addr >= 0x310000 && addr < 0x320000) // TODO
    {
        LOG(if(flags & Log) { fprintf(out, "%X\tSet word in IKAT at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return;
    }

    if(addr >= 0x320000 && addr < 0x324000)
    {
        const uint8_t d = data >> 8;
        timekeeper.SetByte((addr - 0x320000) >> 1, d);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet word in timekeeper at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)d, d, d); })
        return;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tSet word OUT OF RANGE at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
    throw SCC68070Exception(BusError, 0);
}

void Mono3::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        mcd212.SetLong(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet long in VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int32_t)data, data, data); })
        return;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tSet long OUT OF RANGE at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int32_t)data, data, data); })
    throw SCC68070Exception(BusError, 0);
}

uint8_t Mono3::CPUGetUART(const uint8_t flags)
{
    int c = uart_in.get();
    LOG(if(flags & Log) { fprintf(out, "%X\tCPU Get UART: %d '%c' 0x%X\n", cpu.currentPC, c, (char)c, c); })
    return (c != EOF) ? c : 0;
}

void Mono3::CPUSetUART(const uint8_t data, const uint8_t flags)
{
    uart_out.write((char*)&data, 1);
    LOG(if(flags & Log) { fprintf(out, "%X\tCPU Set UART: %d %d '%c' 0x%X\n", cpu.currentPC, (int8_t)data, data, (char)data, data); })
}
