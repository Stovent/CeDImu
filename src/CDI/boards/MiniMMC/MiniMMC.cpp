#include "MiniMMC.hpp"
#include "../../common/Callbacks.hpp"
#include "../../cores/M48T08/M48T08.hpp"

MiniMMC::MiniMMC(CDI& cdi, const void* bios, const uint32_t size, const CDIConfig& conf)
    : Board(cdi, "Mini-MMC", conf)
    , masterVDSC(cdi, true, bios, size)
    , slaveVDSC(cdi, false, "\0", 2)
{
    timekeeper = std::make_unique<M48T08>(cdi, conf.initialTime);
    Reset(true);
}

MiniMMC::~MiniMMC()
{
}

void MiniMMC::Reset(const bool resetCPU)
{
    masterVDSC.Reset();
    slaveVDSC.Reset();
    if(resetCPU)
        cpu.Reset();
}

void MiniMMC::IncrementTime(const double ns)
{
    Board::IncrementTime(ns);
    masterVDSC.IncrementTime(ns);
    slaveVDSC.IncrementTime(ns);
}

uint32_t MiniMMC::GetTotalFrameCount()
{
    return masterVDSC.totalFrameCount;
}

const OS9::BIOS& MiniMMC::GetBIOS() const
{
    return masterVDSC.BIOS;
}

std::vector<InternalRegister> MiniMMC::GetInternalRegisters()
{
    std::vector<InternalRegister> slaveRegs = slaveVDSC.GetInternalRegisters();
    std::vector<InternalRegister> masterRegs = masterVDSC.GetInternalRegisters();
    masterRegs.insert(masterRegs.begin(), slaveRegs.begin(), slaveRegs.end());
    return masterRegs;
}

std::vector<InternalRegister> MiniMMC::GetControlRegisters()
{
    std::vector<InternalRegister> slaveRegs = slaveVDSC.GetControlRegisters();
    std::vector<InternalRegister> masterRegs = masterVDSC.GetControlRegisters();
    masterRegs.insert(masterRegs.begin(), slaveRegs.begin(), slaveRegs.end());
    return masterRegs;
}

uint32_t MiniMMC::GetRAMSize() const
{
    return 0x100000;
}

RAMBank MiniMMC::GetRAMBank1() const
{
    return masterVDSC.GetRAMBank1();
}

RAMBank MiniMMC::GetRAMBank2() const
{
    return slaveVDSC.GetRAMBank2();
}

const Plane& MiniMMC::GetScreen()
{
    return masterVDSC.GetScreen();
}

const Plane& MiniMMC::GetPlaneA()
{
    return masterVDSC.GetPlaneA();
}

const Plane& MiniMMC::GetPlaneB()
{
    return masterVDSC.GetPlaneB();
}

const Plane& MiniMMC::GetBackground()
{
    return masterVDSC.GetBackground();
}

const Plane& MiniMMC::GetCursor()
{
    return masterVDSC.GetCursor();
}
