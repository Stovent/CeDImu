#ifndef MONO3_HPP
#define MONO3_HPP

class Mono3;

#include "../Board.hpp"
#include "../../cores/MCD212/MCD212.hpp"

class Mono3 : public Board
{
    MCD212 mcd212;
    const uint32_t nvramMaxAddress;

public:
    Mono3(CDI& cdi, const void* vdscBios, const uint32_t vdscSize, const void* nvram, const CDIConfig& conf);
    virtual ~Mono3();
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

#endif // MONO3_HPP
