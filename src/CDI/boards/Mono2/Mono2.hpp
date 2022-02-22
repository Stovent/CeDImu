#ifndef CDI_BOARDS_MONO2_MONO2_HPP
#define CDI_BOARDS_MONO2_MONO2_HPP

#include "../Board.hpp"
#include "../../cores/MCD212/MCD212.hpp"

class Mono2 : public Board
{
    MCD212 mcd212;
    const uint32_t nvramMaxAddress;

public:
    Mono2() = delete;
    Mono2(const Mono2&) = delete;
    Mono2(CDI& cdi, const void* vdscBios, const uint32_t vdscSize, const void* nvram, const CDIConfig& conf);
    virtual ~Mono2();
    virtual void Reset(const bool resetCPU) override;
    virtual void IncrementTime(const double ns) override;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Trigger | Log) override;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Trigger | Log) override;
    virtual uint32_t GetLong(const uint32_t addr, const uint8_t flags = Trigger | Log) override;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Trigger | Log) override;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Trigger | Log) override;
    virtual void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Trigger | Log) override;

    virtual uint32_t GetRAMSize() const override;
    virtual RAMBank GetRAMBank1() const override;
    virtual RAMBank GetRAMBank2() const override;

    virtual uint32_t GetTotalFrameCount() override;
    virtual const OS9::BIOS& GetBIOS() const override;

    virtual std::vector<InternalRegister> GetInternalRegisters() override;
    virtual std::vector<InternalRegister> GetControlRegisters() override;
    virtual const Plane& GetScreen() override;
    virtual const Plane& GetPlaneA() override;
    virtual const Plane& GetPlaneB() override;
    virtual const Plane& GetBackground() override;
    virtual const Plane& GetCursor() override;
};

#endif // CDI_BOARDS_MONO2_MONO2_HPP
