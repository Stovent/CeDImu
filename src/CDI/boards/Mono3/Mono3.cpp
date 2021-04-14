#include "Mono3.hpp"
#include "../../common/utils.hpp"
#include "../../HLE/IKAT/IKAT.hpp"

Mono3::Mono3(const void* vdscBios, const uint32_t vdscSize, std::tm* initialTime, const bool PAL) : Board(initialTime), mcd212(*this, vdscBios, vdscSize, PAL)
{
    slave = std::make_unique<HLE::IKAT>(PAL);
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
    if(resetCPU)
        cpu.Reset();
}

void Mono3::ExecuteVideoLine()
{
    mcd212.ExecuteVideoLine();
}

uint32_t Mono3::GetLineDisplayTime()
{
    return mcd212.GetLineDisplayTime();
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

uint32_t Mono3::GetTotalFrameCount()
{
    return mcd212.totalFrameCount;
}

void Mono3::SetOnFrameCompletedCallback(std::function<void()> callback)
{
    mcd212.SetOnFrameCompletedCallback(callback);
}

std::string Mono3::GetPositionInformation(const uint32_t offset)
{
    return mcd212.BIOS.GetPositionInformation(offset - 0x400000);
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

uint32_t Mono3::GetRAMSize() const
{
    return 0x100000;
}

RAMBank Mono3::GetRAMBank1() const
{
    return mcd212.GetRAMBank1();
}

RAMBank Mono3::GetRAMBank2() const
{
    return mcd212.GetRAMBank2();
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
