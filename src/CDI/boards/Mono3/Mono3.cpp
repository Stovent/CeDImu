#include "Mono3.hpp"
#include "../../common/utils.hpp"
#include "../../HLE/IKAT/IKAT.hpp"

Mono3::Mono3(CDI& cdi, const void* vdscBios, const uint32_t vdscSize, const CDIConfig& conf) :
    Board(cdi, "Mono-III", conf),
    mcd212(cdi, vdscBios, vdscSize, conf.PAL)
{
    slave = std::make_unique<HLE::IKAT>(cpu, conf.PAL);
    OPEN_LOG(out, "Mono3.txt")
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

uint32_t Mono3::GetTotalFrameCount()
{
    return mcd212.totalFrameCount;
}

const OS9::BIOS& Mono3::GetBIOS() const
{
    return mcd212.BIOS;
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

const Plane& Mono3::GetScreen()
{
    return mcd212.GetScreen();
}

const Plane& Mono3::GetPlaneA()
{
    return mcd212.GetPlaneA();
}

const Plane& Mono3::GetPlaneB()
{
    return mcd212.GetPlaneB();
}

const Plane& Mono3::GetBackground()
{
    return mcd212.GetBackground();
}

const Plane& Mono3::GetCursor()
{
    return mcd212.GetCursor();
}
