#include "Mono2.hpp"

#include "../../HLE/IKAT/IKAT.hpp"
#include "../../cores/DS1216/DS1216.hpp"
#include "../../cores/M48T08/M48T08.hpp"

Mono2::Mono2(CDI& cdi, const void* vdscBios, const uint32_t vdscSize, const void* nvram, const CDIConfig& conf)
    : Board(cdi, "Mono-II", conf)
    , mcd212(cdi, vdscBios, vdscSize, conf.PAL)
    , nvramMaxAddress(conf.has32KBNVRAM ? 0x330000 : 0x324000)
{
    slave = std::make_unique<HLE::IKAT>(cdi, conf.PAL, 0x310000, PointingDevice::Type::Maneuvering);
    if(conf.has32KBNVRAM)
        timekeeper = std::make_unique<DS1216>(cdi, conf.initialTime, (uint8_t*)nvram);
    else
        timekeeper = std::make_unique<M48T08>(cdi, conf.initialTime, (uint8_t*)nvram);
    Reset(true);
}

Mono2::~Mono2()
{
}

void Mono2::Reset(const bool resetCPU)
{
    mcd212.Reset();
    if(resetCPU)
        cpu.Reset();
}

void Mono2::IncrementTime(const double ns)
{
    Board::IncrementTime(ns);
    mcd212.IncrementTime(ns);
}

uint32_t Mono2::GetTotalFrameCount()
{
    return mcd212.totalFrameCount;
}

const OS9::BIOS& Mono2::GetBIOS() const
{
    return mcd212.BIOS;
}

std::vector<InternalRegister> Mono2::GetInternalRegisters()
{
    return mcd212.GetInternalRegisters();
}

std::vector<InternalRegister> Mono2::GetControlRegisters()
{
    return mcd212.GetControlRegisters();
}

uint32_t Mono2::GetRAMSize() const
{
    return 0x100000;
}

RAMBank Mono2::GetRAMBank1() const
{
    return mcd212.GetRAMBank1();
}

RAMBank Mono2::GetRAMBank2() const
{
    return mcd212.GetRAMBank2();
}

const Plane& Mono2::GetScreen()
{
    return mcd212.GetScreen();
}

const Plane& Mono2::GetPlaneA()
{
    return mcd212.GetPlaneA();
}

const Plane& Mono2::GetPlaneB()
{
    return mcd212.GetPlaneB();
}

const Plane& Mono2::GetBackground()
{
    return mcd212.GetBackground();
}

const Plane& Mono2::GetCursor()
{
    return mcd212.GetCursor();
}
