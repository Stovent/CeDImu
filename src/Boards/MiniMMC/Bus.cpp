#include "MiniMMC.hpp"
#include "../../utils.hpp"

uint8_t MiniMMC::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        uint8_t data = masterVDSC->GetByte(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return data;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        uint8_t data = slaveVDSC->GetByte(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu->currentPC << "\tGet byte OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

uint16_t MiniMMC::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        uint16_t data = masterVDSC->GetWord(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        uint16_t data = slaveVDSC->GetWord(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu->currentPC << "\tGet word OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

uint32_t MiniMMC::GetLong(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        uint32_t data = masterVDSC->GetLong(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        uint32_t data = slaveVDSC->GetLong(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu->currentPC << "\tGet long OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

void MiniMMC::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        masterVDSC->SetByte(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        slaveVDSC->SetByte(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu->currentPC << "\tSet byte OUT OF RANGE at 0x" << addr << std::endl)
}

void MiniMMC::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        masterVDSC->SetWord(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        slaveVDSC->SetWord(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu->currentPC << "\tSet word OUT OF RANGE at 0x" << addr << std::endl)
}

void MiniMMC::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        masterVDSC->SetLong(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        slaveVDSC->SetLong(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu->currentPC << "\tSet long OUT OF RANGE at 0x" << addr << std::endl)
}

uint8_t MiniMMC::CPUGetUART(const uint8_t flags)
{
    int c = uart_in.get();
    LOG(out << std::hex << cpu->currentPC << "\tCPU Get UART: 0x" << c << std::endl)
    return (c != EOF) ? c : 0;
}

void MiniMMC::CPUSetUART(const uint8_t data, const uint8_t flags)
{
    uart_out.write((char*)&data, 1);
    LOG(out << std::hex << cpu->currentPC << "\tCPU Set UART: 0x" << (uint32_t)data << std::endl)
}
