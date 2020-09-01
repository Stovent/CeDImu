#include "MiniMMC.hpp"

#include "../utils.hpp"

MiniMMC::MiniMMC(const void* bios, const uint32_t size) : Board()
{
    masterVDSC = new SCC66470(this, true);
    masterVDSC->LoadBIOS(bios, size);
    slaveVDSC = new SCC66470(this, false);
    cpu = new SCC68070(this);

    OPEN_LOG(out, "MiniMMC.txt")
    uart_out.open("uart_out", std::ios::binary | std::ios::out);
    uart_in.open("uart_in", std::ios::binary | std::ios::in);
}

MiniMMC::~MiniMMC()
{
    delete cpu;
    delete masterVDSC;
    delete slaveVDSC;
}

void MiniMMC::Reset()
{
    masterVDSC->Reset();
    slaveVDSC->Reset();
}

uint8_t MiniMMC::GetByte(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        uint8_t data = masterVDSC->GetByte(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return data;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        uint8_t data = slaveVDSC->GetByte(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu->currentPC << "\tGet byte OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

uint16_t MiniMMC::GetWord(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        uint16_t data = masterVDSC->GetWord(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        uint16_t data = slaveVDSC->GetWord(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu->currentPC << "\tGet word OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

uint32_t MiniMMC::GetLong(const uint32_t addr, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        uint32_t data = masterVDSC->GetLong(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        uint32_t data = slaveVDSC->GetLong(addr, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tGet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return data;
    }

    LOG(out << std::hex << cpu->currentPC << "\tGet long OUT OF RANGE at 0x" << addr << std::endl)
    return 0;
}

void MiniMMC::SetByte(const uint32_t addr, const uint8_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        masterVDSC->SetByte(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        slaveVDSC->SetByte(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet byte at 0x" << addr << " : (0x" << (uint16_t)data << ") " << std::dec << (uint16_t)data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu->currentPC << "\tSet byte OUT OF RANGE at 0x" << addr << std::endl)
}

void MiniMMC::SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        masterVDSC->SetWord(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        slaveVDSC->SetWord(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet word at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu->currentPC << "\tSet word OUT OF RANGE at 0x" << addr << std::endl)
}

void MiniMMC::SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags)
{
    if(addr < 0x080000 || (addr >= 0x180000 && addr < 0x1FFC00) || (addr >= 0x1FFFE0 && addr < 0x200000))
    {
        masterVDSC->SetLong(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    if((addr >= 0x080000 && addr < 0x100000) || (addr >= 0x1FFFC0 && addr < 0x1FFFE0))
    {
        slaveVDSC->SetLong(addr, data, flags);
        LOG(if(flags & Log) { out << std::hex << cpu->currentPC << "\tSet long at 0x" << addr << " : (0x" << data << ") " << std::dec << data << std::endl;} )
        return;
    }

    LOG(out << std::hex << cpu->currentPC << "\tSet long OUT OF RANGE at 0x" << addr << std::endl)
}

uint8_t MiniMMC::CPUGetUART(const uint8_t flags)
{
    int c = uart_in.get();
    LOG(out << std::hex << cpu->currentPC << "\tCPU Get UART: 0x" << c << std::endl)
    return (c != EOF) ? c : 0;
}

void MiniMMC::CPUSetUART(const uint8_t data, const uint8_t flags)
{
    uart_out.write((char*)&data, 1);
    LOG(out << std::hex << cpu->currentPC << "\tCPU Set UART: 0x" << (uint32_t)data << std::endl)
}

void MiniMMC::DrawLine()
{
    masterVDSC->DrawLine();
    slaveVDSC->DrawLine();
}

uint32_t MiniMMC::GetLineDisplayTime()
{
    return masterVDSC->GetLineDisplayTimeNanoSeconds();
}

void MiniMMC::PutDataInMemory(const void* s, unsigned int size, unsigned int position)
{
    if(position >= 0x080000)
        slaveVDSC->PutDataInMemory(s, size, position);
    else
        masterVDSC->PutDataInMemory(s, size, position);
}

void MiniMMC::StopOnNextFrame(const bool stop)
{
    masterVDSC->StopOnNextFrame(stop);
}

uint32_t MiniMMC::GetAllocatedMemory()
{
    return masterVDSC->allocatedMemory;
}

uint32_t MiniMMC::GetTotalFrameCount()
{
    return masterVDSC->totalFrameCount;
}

void MiniMMC::SetOnFrameCompletedCallback(std::function<void()> callback)
{
    masterVDSC->SetOnFrameCompletedCallback(callback);
    slaveVDSC->SetOnFrameCompletedCallback(callback);
}

std::vector<VDSCRegister> MiniMMC::GetInternalRegisters()
{
    std::vector<VDSCRegister> slaveRegs = slaveVDSC->GetInternalRegisters();
    std::vector<VDSCRegister> masterRegs = masterVDSC->GetInternalRegisters();
    masterRegs.insert(masterRegs.begin(), slaveRegs.begin(), slaveRegs.end());
    return masterRegs;
}

std::vector<VDSCRegister> MiniMMC::GetControlRegisters()
{
    std::vector<VDSCRegister> slaveRegs = slaveVDSC->GetControlRegisters();
    std::vector<VDSCRegister> masterRegs = masterVDSC->GetControlRegisters();
    masterRegs.insert(masterRegs.begin(), slaveRegs.begin(), slaveRegs.end());
    return masterRegs;
}

std::vector<std::string> MiniMMC::GetICA1()
{
    std::vector<std::string> ca;
    ca.push_back("Master:");
    ca.insert(ca.end(), masterVDSC->ICA1.begin(), masterVDSC->ICA1.end());
    ca.push_back("Slave:");
    ca.insert(ca.end(), slaveVDSC->ICA1.begin(), slaveVDSC->ICA1.end());
    return ca;
}

std::vector<std::string> MiniMMC::GetDCA1()
{
    std::vector<std::string> ca;
    ca.push_back("Master:");
    ca.insert(ca.end(), masterVDSC->DCA1.begin(), masterVDSC->DCA1.end());
    ca.push_back("Slave:");
    ca.insert(ca.end(), slaveVDSC->DCA1.begin(), slaveVDSC->DCA1.end());
    return ca;
}

std::vector<std::string> MiniMMC::GetICA2()
{
    std::vector<std::string> ca;
    ca.push_back("Master:");
    ca.insert(ca.end(), masterVDSC->ICA2.begin(), masterVDSC->ICA2.end());
    ca.push_back("Slave:");
    ca.insert(ca.end(), slaveVDSC->ICA2.begin(), slaveVDSC->ICA2.end());
    return ca;
}

std::vector<std::string> MiniMMC::GetDCA2()
{
    std::vector<std::string> ca;
    ca.push_back("Master:");
    ca.insert(ca.end(), masterVDSC->DCA2.begin(), masterVDSC->DCA2.end());
    ca.push_back("Slave:");
    ca.insert(ca.end(), slaveVDSC->DCA2.begin(), slaveVDSC->DCA2.end());
    return ca;
}

wxImage MiniMMC::GetScreen()
{
    return masterVDSC->GetScreen();
}

wxImage MiniMMC::GetPlaneA()
{
    return masterVDSC->GetPlaneA();
}

wxImage MiniMMC::GetPlaneB()
{
    return masterVDSC->GetPlaneB();
}

wxImage MiniMMC::GetBackground()
{
    return masterVDSC->GetBackground();
}

wxImage MiniMMC::GetCursor()
{
    return masterVDSC->GetCursor();
}
