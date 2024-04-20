#ifndef CDI_BOARDS_MONO3_MONO3_HPP
#define CDI_BOARDS_MONO3_MONO3_HPP

#include "../../CDI.hpp"
#include "../../cores/MCD212/MCD212.hpp"
#include "../../HLE/CIAP/CIAP.hpp"

#include <span>

class Mono3 : public CDI
{
public:
    Mono3(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc = CDIDisc(), std::string_view boardName = "Mono-III");
    virtual ~Mono3() noexcept;

    virtual void Scheduler() override;

    virtual void Reset(bool resetCPU) override;

    virtual uint8_t  PeekByte(uint32_t addr) const noexcept override;
    virtual uint16_t PeekWord(uint32_t addr) const noexcept override;
    virtual uint32_t PeekLong(uint32_t addr) const noexcept override;

    virtual uint8_t  GetByte(uint32_t addr, BusFlags flags) override;
    virtual uint16_t GetWord(uint32_t addr, BusFlags flags) override;
    virtual uint32_t GetLong(uint32_t addr, BusFlags flags) override;

    virtual void SetByte(uint32_t addr, uint8_t  data, BusFlags flags) override;
    virtual void SetWord(uint32_t addr, uint16_t data, BusFlags flags) override;
    virtual void SetLong(uint32_t addr, uint32_t data, BusFlags flags) override;

    virtual uint32_t GetRAMSize() const override;
    virtual RAMBank GetRAMBank1() const override;
    virtual RAMBank GetRAMBank2() const override;

    virtual uint32_t GetTotalFrameCount() override;
    virtual const OS9::BIOS& GetBIOS() const override;
    virtual uint32_t GetBIOSBaseAddress() const override;

    virtual std::vector<InternalRegister> GetVDSCInternalRegisters() override;
    virtual std::vector<InternalRegister> GetVDSCControlRegisters() override;
    virtual const Video::Plane& GetScreen() override;
    virtual const Video::Plane& GetPlaneA() override;
    virtual const Video::Plane& GetPlaneB() override;
    virtual const Video::Plane& GetBackground() override;
    virtual const Video::Plane& GetCursor() override;

private:
    MCD212 m_mcd212;
    HLE::CIAP m_ciap;
    const uint32_t m_nvramMaxAddress;
};

#endif // CDI_BOARDS_MONO3_MONO3_HPP
