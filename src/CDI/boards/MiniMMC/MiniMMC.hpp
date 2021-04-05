#ifndef MINIMMC_HPP
#define MINIMMC_HPP

class MiniMMC;

#include "../Board.hpp"
#include "../../cores/SCC66470/SCC66470.hpp"

#include <cstdio>
#include <fstream>

class MiniMMC : public Board
{
    SCC66470 masterVDSC;
    SCC66470 slaveVDSC;
    FILE* out;
    std::ofstream uart_out;
    std::ifstream uart_in;

public:
    MiniMMC(const void* bios, const uint32_t size, const bool initNVRAMClock);
    virtual ~MiniMMC();
    virtual void Reset(const bool resetCPU) override;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Trigger | Log) override;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Trigger | Log) override;
    virtual uint32_t GetLong(const uint32_t addr, const uint8_t flags = Trigger | Log) override;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Trigger | Log) override;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Trigger | Log) override;
    virtual void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Trigger | Log) override;

    virtual uint8_t CPUGetUART(const uint8_t flags = Trigger | Log) override;
    virtual void CPUSetUART(const uint8_t data, const uint8_t flags = Trigger | Log) override;

    virtual void ExecuteVideoLine() override;
    virtual uint32_t GetLineDisplayTime() override;

    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) override;
    virtual void WriteToBIOSArea(const void* s, unsigned int size, unsigned int position) override;
    virtual void StopOnNextFrame(const bool stop = true) override;

    virtual uint32_t GetAllocatedMemory() override;
    virtual uint32_t GetTotalFrameCount() override;
    virtual void SetOnFrameCompletedCallback(std::function<void()> callback) override;
    virtual std::string GetPositionInformation(const uint32_t offset) override;

    virtual std::vector<std::string> GetICA1() override;
    virtual std::vector<std::string> GetDCA1() override;
    virtual std::vector<std::string> GetICA2() override;
    virtual std::vector<std::string> GetDCA2() override;

    virtual std::vector<VDSCRegister> GetInternalRegisters() override;
    virtual std::vector<VDSCRegister> GetControlRegisters() override;
    virtual Plane GetScreen() override;
    virtual Plane GetPlaneA() override;
    virtual Plane GetPlaneB() override;
    virtual Plane GetBackground() override;
    virtual Plane GetCursor() override;
};

#endif // MINIMMC_HPP
