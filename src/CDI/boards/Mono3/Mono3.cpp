#include "Mono3.hpp"
#include "../../common/utils.hpp"

Mono3::Mono3(const void* vdscBios, const uint32_t vdscSize, const void* slaveBios, const uint16_t slaveSize, const bool initNVRAMClock) : Board(slaveBios, slaveSize, initNVRAMClock), mcd212(this)
{
    mcd212.LoadBIOS(vdscBios, vdscSize);

    OPEN_LOG(out, "Mono3.txt")
    uart_out.open("uart_out", std::ios::binary | std::ios::out);
    uart_in.open("uart_in", std::ios::binary | std::ios::in);
    Reset(true);
}

Mono3::~Mono3()
{
    CLOSE_LOG(out)
}

void Mono3::Reset(const bool resetCPU)
{
    mcd212.Reset();
    slave.Reset();
    if(resetCPU)
        cpu.Reset();
}

uint8_t Mono3::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        const uint8_t data = mcd212.GetByte(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet byte in VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    if(addr >= 0x320000 && addr < 0x324000)
    {
        const uint8_t data = timekeeper.GetByte((addr - 0x320000) >> 1);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet byte in timekeeper at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return data;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tGet byte OUT OF RANGE at 0x%X\n", cpu.currentPC, addr); })
    return 0;
}

uint16_t Mono3::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        const uint16_t data = mcd212.GetWord(addr, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tGet word in VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return data;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tGet word OUT OF RANGE at 0x%X\n", cpu.currentPC, addr); })
    return 0;
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
    return 0;
}

void Mono3::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        mcd212.SetByte(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet byte in VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    if(addr >= 0x320000 && addr < 0x324000)
    {
        timekeeper.SetByte((addr - 0x320000) >> 1, data);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet byte in timekeeper at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
        return;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tSet byte OUT OF RANGE at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int8_t)data, data, data); })
}

void Mono3::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x200000 && addr < 0x280000) || (addr >= 0x400000 && addr < 0x500000))
    {
        mcd212.SetWord(addr, data, flags);
        LOG(if(flags & Log) { fprintf(out, "%X\tSet word in VDSC at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
        return;
    }

    LOG(if(flags & Log) { fprintf(out, "%X\tSet word OUT OF RANGE at 0x%X : %d %d 0x%X\n", cpu.currentPC, addr, (int16_t)data, data, data); })
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

Plane Mono3::GetScreen()
{
    return mcd212.GetScreen();
}

Plane Mono3::GetPlaneA()
{
    return mcd212.GetPlaneA();
}

Plane Mono3::GetPlaneB()
{
    return mcd212.GetPlaneB();
}

Plane Mono3::GetBackground()
{
    return mcd212.GetBackground();
}

Plane Mono3::GetCursor()
{
    return mcd212.GetCursor();
}
