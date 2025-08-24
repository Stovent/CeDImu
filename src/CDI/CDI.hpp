#ifndef CDI_CDI_HPP
#define CDI_CDI_HPP

#include "CDIConfig.hpp"
#include "CDIDisc.hpp"
#include "common/Callbacks.hpp"
#include "cores/IRTC.hpp"
#include "cores/ISlave.hpp"
#include "cores/SCC68070/SCC68070.hpp"
#include "OS9/BIOS.hpp"
#include "OS9/Kernel.hpp"

#include <memory>
#include <span>
#include <string_view>

class Mono3;

/** \brief Base class for a CD-i player.
 */
class CDI
{
public:
    const std::string m_boardName; /**< Name of the hardware architecture (see http://icdia.co.uk/players/comparison.html). */
    const CDIConfig m_config; /**< Configuration of the CDI context. */
    CDIDisc m_disc; /**< CDI disc. */
    Callbacks m_callbacks; /**< The user callbacks. */

    SCC68070 m_cpu; /**< The main CPU. */
    std::unique_ptr<ISlave> m_slave; /**< The slave processor. */
    std::unique_ptr<IRTC> m_timekeeper; /**< The NVRAM chip. */

    static std::unique_ptr<CDI> NewCDI(Boards board, std::span<const uint8_t> systemBios, std::span<const uint8_t> nvram, CDIConfig config = defaultConfig, Callbacks callbacks = Callbacks(), CDIDisc disc = CDIDisc());
    static std::unique_ptr<CDI> NewMono3(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config = defaultConfig, Callbacks callbacks = Callbacks(), CDIDisc disc = CDIDisc());
    static std::unique_ptr<CDI> NewMono4(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config = defaultConfig, Callbacks callbacks = Callbacks(), CDIDisc disc = CDIDisc());
    static std::unique_ptr<CDI> NewRoboco(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config = defaultConfig, Callbacks callbacks = Callbacks(), CDIDisc disc = CDIDisc());

    virtual ~CDI() noexcept;

    CDI(const CDI&) = delete;
    CDI& operator=(const CDI&) = delete;

    CDI(CDI&&) = delete;
    CDI& operator=(CDI&&) = delete;

    virtual uint32_t GetRAMSize() const = 0;
    virtual RAMBank GetRAMBank1() const = 0;
    virtual RAMBank GetRAMBank2() const = 0;

    /** \brief Returns a pointer to the given address.
     * The returned pointer is only valid for the given memory bank and must not be assumed to be consecutive with all the memory map.
     * Specifically, RAM bank 1 and 2 may be non-consecutive, and ROM is very likely allocated separately.
     */
    virtual const uint8_t* GetPointer(uint32_t addr) const;

    virtual uint8_t  PeekByte(uint32_t addr) const noexcept = 0;
    virtual uint16_t PeekWord(uint32_t addr) const noexcept = 0;
    virtual uint32_t PeekLong(uint32_t addr) const noexcept = 0;

    virtual uint32_t GetTotalFrameCount() = 0;
    virtual const OS9::BIOS& GetBIOS() const = 0;
    virtual uint32_t GetBIOSBaseAddress() const = 0;
    const OS9::Kernel* GetKernel() const;
    std::string GetModuleNameAt(uint32_t addr) const;

    virtual std::vector<InternalRegister> GetVDSCInternalRegisters() = 0;
    virtual std::vector<InternalRegister> GetVDSCControlRegisters() = 0;
    virtual const Video::Plane& GetScreen() = 0;
    virtual const Video::Plane& GetPlaneA() = 0;
    virtual const Video::Plane& GetPlaneB() = 0;
    virtual const Video::Plane& GetBackground() = 0;
    virtual const Video::Plane& GetCursor() = 0;


protected:
    friend Mono3;
    friend SCC68070;

    OS9::EmulatedMemoryAccess GetEmulatedMemoryAccessCallbacks() const;
    OS9::Kernel m_kernel{GetEmulatedMemoryAccessCallbacks()};

    CDI() = delete;
    CDI(std::string_view boardName, CDIConfig config, Callbacks callbacks, CDIDisc disc = CDIDisc());

    virtual void Reset(bool resetCPU) = 0;
    virtual void IncrementTime(double ns);

    virtual uint8_t  GetByte(uint32_t addr, BusFlags flags) = 0;
    virtual uint16_t GetWord(uint32_t addr, BusFlags flags) = 0;
    virtual uint32_t GetLong(uint32_t addr, BusFlags flags) = 0;

    virtual void SetByte(uint32_t addr, uint8_t  data, BusFlags flags) = 0;
    virtual void SetWord(uint32_t addr, uint16_t data, BusFlags flags) = 0;
    virtual void SetLong(uint32_t addr, uint32_t data, BusFlags flags) = 0;
};

#endif // CDI_CDI_HPP
