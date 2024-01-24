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
    virtual ~Mono3();

    Mono3(const Mono3&) = delete;
    Mono3& operator=(const Mono3&) = delete;

    Mono3(Mono3&&) = delete;
    Mono3& operator=(Mono3&&) = delete;

    virtual void Reset(const bool resetCPU) override;
    virtual void IncrementTime(const double ns) override;
    virtual uint32_t GetBIOSBaseAddress() const override;

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
