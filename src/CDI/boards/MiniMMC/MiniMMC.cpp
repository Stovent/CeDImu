#include "MiniMMC.hpp"
#include "../../common/Callbacks.hpp"

MiniMMC::MiniMMC(CDI& cdi, const void* bios, const uint32_t size, const CDIConfig& conf) :
    Board(cdi, "Mini-MMC", conf),
    masterVDSC(cdi, true, bios, size),
    slaveVDSC(cdi, false, "\0", 2)
{
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

void MiniMMC::ExecuteVideoLine()
{
    masterVDSC.ExecuteVideoLine();
    slaveVDSC.ExecuteVideoLine();
}

uint32_t MiniMMC::GetLineDisplayTime()
{
    return masterVDSC.GetLineDisplayTime();
}

void MiniMMC::PutDataInMemory(const void* s, unsigned int size, unsigned int position)
{
    if(position >= 0x080000)
        slaveVDSC.PutDataInMemory(s, size, position);
    else
        masterVDSC.PutDataInMemory(s, size, position);
}

void MiniMMC::WriteToBIOSArea(const void* s, unsigned int size, unsigned int position)
{
    masterVDSC.WriteToBIOSArea(s, size, position);
}

uint32_t MiniMMC::GetTotalFrameCount()
{
    return masterVDSC.totalFrameCount;
}

const OS9::BIOS& MiniMMC::GetBIOS() const
{
    return masterVDSC.BIOS;
}

std::vector<VDSCRegister> MiniMMC::GetInternalRegisters()
{
    std::vector<VDSCRegister> slaveRegs = slaveVDSC.GetInternalRegisters();
    std::vector<VDSCRegister> masterRegs = masterVDSC.GetInternalRegisters();
    masterRegs.insert(masterRegs.begin(), slaveRegs.begin(), slaveRegs.end());
    return masterRegs;
}

std::vector<VDSCRegister> MiniMMC::GetControlRegisters()
{
    std::vector<VDSCRegister> slaveRegs = slaveVDSC.GetControlRegisters();
    std::vector<VDSCRegister> masterRegs = masterVDSC.GetControlRegisters();
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
