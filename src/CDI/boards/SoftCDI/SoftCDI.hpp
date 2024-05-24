#ifndef CDI_BOARDS_SOFTCDI_SOFTCDI_HPP
#define CDI_BOARDS_SOFTCDI_SOFTCDI_HPP

#include "../../CDI.hpp"

#include <span>

/** \brief Software implementation of the Green Book, with the memory map of a Mono3 board.
 */
class SoftCDI : public CDI
{
public:
    SoftCDI(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc = CDIDisc());
    virtual ~SoftCDI();

    SoftCDI(const SoftCDI&) = delete;
    SoftCDI& operator=(const SoftCDI&) = delete;

    SoftCDI(SoftCDI&&) = delete;
    SoftCDI& operator=(SoftCDI&&) = delete;

    virtual void Scheduler() override;

    virtual void Reset(const bool resetCPU) override;
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
    static constexpr size_t RAM_BANK_SIZE = 0x80000u; // 512KB
    std::vector<uint8_t> m_ram0;
    std::vector<uint8_t> m_ram1;
    OS9::BIOS m_bios;
    // const uint32_t m_nvramMaxAddress;

    // Specifics to allow the BIOS to initialize.
    uint8_t m_csr1r; /**< CSR1R register of MCD212 to emulate Display Active. */
};

#endif // CDI_BOARDS_SOFTCDI_SOFTCDI_HPP
