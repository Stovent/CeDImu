#include "Mono3.hpp"

#include "../utils.hpp"

Mono3::Mono3(const void* bios, const uint32_t size) : Board()
{
    mcd212 = new MCD212(this);
    mcd212->LoadBIOS(bios, size);
    cpu = new SCC68070(this);

    OPEN_LOG(out, "Mono3.txt")
    uart_out.open("uart_out", std::ios::binary | std::ios::out);
    uart_in.open("uart_in", std::ios::binary | std::ios::in);
}

Mono3::~Mono3()
{
    delete cpu;
    delete mcd212;
}

void Mono3::Reset()
{
    mcd212->Reset();
}

uint8_t Mono3::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        uint8_t data = mcd212->GetByte(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu->currentPC << "\tGet byte OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

uint16_t Mono3::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        uint16_t data = mcd212->GetWord(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu->currentPC << "\tGet word OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

uint32_t Mono3::GetLong(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        uint32_t data = mcd212->GetLong(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu->currentPC << "\tGet long OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

void Mono3::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        mcd212->SetByte(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu->currentPC << "\tSet byte OUT OF RANGE at 0x" << addr << std::endl)
}

void Mono3::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        mcd212->SetWord(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu->currentPC << "\tSet word OUT OF RANGE at 0x" << addr << std::endl)
}

void Mono3::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        mcd212->SetLong(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu->currentPC << "\tSet long OUT OF RANGE at 0x" << addr << std::endl)
}

uint8_t Mono3::CPUGetUART(const uint8_t flags)
{
    int c = uart_in.get();
    LOG(out << std::hex << cpu->currentPC << "\tCPU Get UART: 0x" << c << std::endl)
    return (c != EOF) ? c : 0;
}

void Mono3::CPUSetUART(const uint8_t data, const uint8_t flags)
{
    uart_out.write((char*)&data, 1);
    LOG(out << std::hex << cpu->currentPC << "\tCPU Set UART: 0x" << (uint32_t)data << std::endl)
}

void Mono3::DrawLine()
{
    mcd212->DrawLine();
}

uint32_t Mono3::GetLineDisplayTime()
{
    return mcd212->GetLineDisplayTimeNanoSeconds();
}
