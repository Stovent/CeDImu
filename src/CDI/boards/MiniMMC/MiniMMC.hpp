#ifndef CDI_BOARDS_MINIMMC_MINIMMC_HPP
#define CDI_BOARDS_MINIMMC_MINIMMC_HPP

class MiniMMC;

#include "../Board.hpp"
#include "../../cores/SCC66470/SCC66470.hpp"

class MiniMMC : public Board
{
    SCC66470 masterVDSC;
    SCC66470 slaveVDSC;

public:
    MiniMMC(CDI& cdi, const void* bios, const uint32_t size, const CDIConfig& conf);
    virtual ~MiniMMC();
    virtual void Reset(const bool resetCPU) override;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Trigger | Log) override;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Trigger | Log) override;
    virtual uint32_t GetLong(const uint32_t addr, const uint8_t flags = Trigger | Log) override;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Trigger | Log) override;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Trigger | Log) override;
    virtual void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Trigger | Log) override;

    virtual uint32_t GetRAMSize() const override;
    virtual RAMBank GetRAMBank1() const override;
    virtual RAMBank GetRAMBank2() const override;

    virtual void ExecuteVideoLine() override;
    virtual uint32_t GetLineDisplayTime() override;

    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) override;
    virtual void WriteToBIOSArea(const void* s, unsigned int size, unsigned int position) override;

    virtual uint32_t GetTotalFrameCount() override;
    virtual const OS9::BIOS& GetBIOS() const override;

    virtual std::vector<VDSCRegister> GetInternalRegisters() override;
    virtual std::vector<VDSCRegister> GetControlRegisters() override;
    virtual const Plane& GetScreen() override;
    virtual const Plane& GetPlaneA() override;
    virtual const Plane& GetPlaneB() override;
    virtual const Plane& GetBackground() override;
    virtual const Plane& GetCursor() override;
};

#endif // CDI_BOARDS_MINIMMC_MINIMMC_HPP
