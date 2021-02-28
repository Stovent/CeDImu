#include "MiniMMC.hpp"
#include "../../common/utils.hpp"

uint8_t MiniMMC::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        const uint8_t data = masterVDSC.GetByte(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet byte in master VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        const uint8_t data = slaveVDSC.GetByte(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet byte in slave VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    if(addr >= 0x3F8000 && addr < 0x3FC000 && isEven(addr))
    {
        const uint8_t data = timekeeper.GetByte((addr - 0x3F8000) >> 1);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet byte in timekeeper at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tGet byte OUT OF RANGE at 0x%X\n", cpu.currentPC, addr); })
    return 0;
}

uint16_t MiniMMC::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        const uint16_t data = masterVDSC.GetWord(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet word in master VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return data;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        const uint16_t data = slaveVDSC.GetWord(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet word in slave VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return data;
    }

    if(addr >= 0x3F8000 && addr < 0x3FC000)
    {
        const uint8_t data = timekeeper.GetByte((addr - 0x3F8000) >> 1);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet word in timekeeper at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return (uint16_t)data << 8;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tGet word OUT OF RANGE at 0x%X\n", cpu.currentPC, addr); })
    return 0;
}

uint32_t MiniMMC::GetLong(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        const uint32_t data = masterVDSC.GetLong(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet long in master VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int32_t)data, data, data); })
        return data;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        const uint32_t data = slaveVDSC.GetLong(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet long in slave VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int32_t)data, data, data); })
        return data;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tGet long OUT OF RANGE at 0x%X\n", cpu.currentPC, addr); })
    return 0;
}

void MiniMMC::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        masterVDSC.SetByte(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet byte in master VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        slaveVDSC.SetByte(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet byte in slave VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    if(addr >= 0x3F8000 && addr < 0x3FC000 && isEven(addr))
    {
        timekeeper.SetByte((addr - 0x3F8000) >> 1, data);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet byte in timekeeper at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tSet byte OUT OF RANGE at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
}

void MiniMMC::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        masterVDSC.SetWord(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet word in master VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        slaveVDSC.SetWord(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet word in slave VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return;
    }

    if(addr >= 0x3F8000 && addr < 0x3FC000)
    {
        const uint8_t d = data >> 8;
        timekeeper.SetByte((addr - 0x3F8000) >> 1, d);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet word in timekeeper at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)d, d, d); })
        return;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tSet word OUT OF RANGE at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
}

void MiniMMC::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        masterVDSC.SetLong(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet long in master VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int32_t)data, data, data); })
        return;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        slaveVDSC.SetLong(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet long in slave VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int32_t)data, data, data); })
        return;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tSet long OUT OF RANGE at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int32_t)data, data, data); })
}

uint8_t MiniMMC::CPUGetUART(const uint8_t flags)
{
    int c = uart_in.get();
    LOG(if(flags & Log) { fprintf(out, "%X\tCPU Get UART: %d '%c' 0x%X\n", cpu.currentPC, c, (char)c, c); })
    return (c != EOF) ? c : 0;
}

void MiniMMC::CPUSetUART(const uint8_t data, const uint8_t flags)
{
    uart_out.write((char*)&data, 1);
    LOG(if(flags & Log) { fprintf(out, "%X\tCPU Set UART: %d %d '%c' 0x%X\n", cpu.currentPC, (int8_t)data, data, (char)data, data); })
}
