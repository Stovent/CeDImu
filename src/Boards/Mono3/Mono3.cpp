#include "Mono3.hpp"

#include "../../utils.hpp"

Mono3::Mono3(const void* vdscBios, const uint32_t vdscSize, const void* slaveBios, const uint16_t slaveSize) : Board(slaveBios, slaveSize), mcd212(this)
{
    mcd212.LoadBIOS(vdscBios, vdscSize);

    OPEN_LOG(out, "Mono3.txt")
    uart_out.open("uart_out", std::ios::binary | std::ios::out);
    uart_in.open("uart_in", std::ios::binary | std::ios::in);
    Reset();
}

Mono3::~Mono3()
{
}

void Mono3::Reset()
{
    mcd212.Reset();
    slave.Reset();
    cpu.Reset();
}

uint8_t Mono3::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        uint8_t data = mcd212.GetByte(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu.currentPC << "\tGet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu.currentPC << "\tGet byte OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

uint16_t Mono3::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        uint16_t data = mcd212.GetWord(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu.currentPC << "\tGet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu.currentPC << "\tGet word OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

uint32_t Mono3::GetLong(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        uint32_t data = mcd212.GetLong(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu.currentPC << "\tGet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu.currentPC << "\tGet long OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

void Mono3::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        mcd212.SetByte(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu.currentPC << "\tSet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu.currentPC << "\tSet byte OUT OF RANGE at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl)
}

void Mono3::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        mcd212.SetWord(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu.currentPC << "\tSet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu.currentPC << "\tSet word OUT OF RANGE at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl)
}

void Mono3::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || addr >= 0x400000)
    {
        mcd212.SetLong(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu.currentPC << "\tSet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu.currentPC << "\tSet long OUT OF RANGE at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl)
}

uint8_t Mono3::CPUGetUART(const uint8_t flags)
{
    int c = uart_in.get();
    LOG(out << std::hex << cpu.currentPC << "\tCPU Get UART: 0x" << c << std::endl)
    return (c != EOF) ? c : 0;
}

void Mono3::CPUSetUART(const uint8_t data, const uint8_t flags)
{
    uart_out.write((char*)&data, 1);
    LOG(out << std::hex << cpu.currentPC << "\tCPU Set UART: 0x" << (uint32_t)data << std::endl)
}

void Mono3::DrawLine()
{
    mcd212.DrawLine();
}

uint32_t Mono3::GetLineDisplayTime()
{
    return mcd212.GetLineDisplayTimeNanoSeconds();
}

void Mono3::PutDataInMemory(const void* s, unsigned int size, unsigned int position)
{
    mcd212.PutDataInMemory(s, size, position);
}

void Mono3::WriteToBIOSArea(const void* s, unsigned int size, unsigned int position)
{
    mcd212.WriteToBIOSArea(s, size, position);
}

void Mono3::StopOnNextFrame(const bool stop)
{
    mcd212.StopOnNextFrame(stop);
}

uint32_t Mono3::GetAllocatedMemory()
{
    return mcd212.allocatedMemory;
}

uint32_t Mono3::GetTotalFrameCount()
{
    return mcd212.totalFrameCount;
}

void Mono3::SetOnFrameCompletedCallback(std::function<void()> callback)
{
    mcd212.SetOnFrameCompletedCallback(callback);
}

std::vector<std::string> Mono3::GetICA1()
{
    return mcd212.ICA1;
}

std::vector<std::string> Mono3::GetDCA1()
{
    return mcd212.DCA1;
}

std::vector<std::string> Mono3::GetICA2()
{
    return mcd212.ICA2;
}

std::vector<std::string> Mono3::GetDCA2()
{
    return mcd212.DCA2;
}

std::vector<VDSCRegister> Mono3::GetInternalRegisters()
{
    return mcd212.GetInternalRegisters();
}

std::vector<VDSCRegister> Mono3::GetControlRegisters()
{
    return mcd212.GetControlRegisters();
}

wxImage Mono3::GetScreen()
{
    return mcd212.GetScreen();
}

wxImage Mono3::GetPlaneA()
{
    return mcd212.GetPlaneA();
}

wxImage Mono3::GetPlaneB()
{
    return mcd212.GetPlaneB();
}

wxImage Mono3::GetBackground()
{
    return mcd212.GetBackground();
}

wxImage Mono3::GetCursor()
{
    return mcd212.GetCursor();
}
