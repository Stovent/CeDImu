#include "MiniMMC.hpp"
#include "../../common/utils.hpp"

MiniMMC::MiniMMC(const void* bios, const uint32_t size, const bool initNVRAMClock) : Board(nullptr, 0, initNVRAMClock), masterVDSC(this, true), slaveVDSC(this, false)
{
    masterVDSC.LoadBIOS(bios, size);

    OPEN_LOG(out, "MiniMMC.txt")
    uart_out.open("uart_out", std::ios::binary | std::ios::out);
    uart_in.open("uart_in", std::ios::binary | std::ios::in);
    Reset();
}

MiniMMC::~MiniMMC()
{
}

void MiniMMC::Reset()
{
    masterVDSC.Reset();
    slaveVDSC.Reset();
    cpu.Reset();
}

void MiniMMC::DrawLine()
{
    masterVDSC.DrawLine();
    slaveVDSC.DrawLine();
}

uint32_t MiniMMC::GetLineDisplayTime()
{
    return masterVDSC.GetLineDisplayTimeNanoSeconds();
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

void MiniMMC::StopOnNextFrame(const bool stop)
{
    masterVDSC.StopOnNextFrame(stop);
}

uint32_t MiniMMC::GetAllocatedMemory()
{
    return masterVDSC.allocatedMemory;
}

uint32_t MiniMMC::GetTotalFrameCount()
{
    return masterVDSC.totalFrameCount;
}

void MiniMMC::SetOnFrameCompletedCallback(std::function<void()> callback)
{
    masterVDSC.SetOnFrameCompletedCallback(callback);
    slaveVDSC.SetOnFrameCompletedCallback(callback);
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
