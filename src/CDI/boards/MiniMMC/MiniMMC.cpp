#include "MiniMMC.hpp"
#include "../../common/utils.hpp"

MiniMMC::MiniMMC(CDI& cdi, const void* bios, const uint32_t size, const CDIConfig& conf) :
    Board(cdi, "Mini-MMC", conf),
    masterVDSC(cdi, true, bios, size),
    slaveVDSC(cdi, false, "\0", 2)
{
    OPEN_LOG(out, "MiniMMC.txt")
    Reset(true);
}

MiniMMC::~MiniMMC()
{
    CLOSE_LOG(out)
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

std::vector<std::string> MiniMMC::GetICA1()
{
    std::vector<std::string> ca;
    ca.push_back("Master:");
    ca.insert(ca.end(), masterVDSC.ICA1.begin(), masterVDSC.ICA1.end());
    ca.push_back("Slave:");
    ca.insert(ca.end(), slaveVDSC.ICA1.begin(), slaveVDSC.ICA1.end());
    return ca;
}

std::vector<std::string> MiniMMC::GetDCA1()
{
    std::vector<std::string> ca;
    ca.push_back("Master:");
    ca.insert(ca.end(), masterVDSC.DCA1.begin(), masterVDSC.DCA1.end());
    ca.push_back("Slave:");
    ca.insert(ca.end(), slaveVDSC.DCA1.begin(), slaveVDSC.DCA1.end());
    return ca;
}

std::vector<std::string> MiniMMC::GetICA2()
{
    std::vector<std::string> ca;
    ca.push_back("Master:");
    ca.insert(ca.end(), masterVDSC.ICA2.begin(), masterVDSC.ICA2.end());
    ca.push_back("Slave:");
    ca.insert(ca.end(), slaveVDSC.ICA2.begin(), slaveVDSC.ICA2.end());
    return ca;
}

std::vector<std::string> MiniMMC::GetDCA2()
{
    std::vector<std::string> ca;
    ca.push_back("Master:");
    ca.insert(ca.end(), masterVDSC.DCA2.begin(), masterVDSC.DCA2.end());
    ca.push_back("Slave:");
    ca.insert(ca.end(), slaveVDSC.DCA2.begin(), slaveVDSC.DCA2.end());
    return ca;
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

Plane MiniMMC::GetScreen()
{
    return masterVDSC.GetScreen();
}

Plane MiniMMC::GetPlaneA()
{
    return masterVDSC.GetPlaneA();
}

Plane MiniMMC::GetPlaneB()
{
    return masterVDSC.GetPlaneB();
}

Plane MiniMMC::GetBackground()
{
    return masterVDSC.GetBackground();
}

Plane MiniMMC::GetCursor()
{
    return masterVDSC.GetCursor();
}
