#include "SoftCDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../HLE/IKAT/IKAT.hpp"
#include "../../cores/DS1216/DS1216.hpp"
#include "../../cores/M48T08/M48T08.hpp"
#include "../../SoftCDI/include/NVDRV.h"
#include "../../SoftCDI/include/VIDEO.h"
#include "../../SoftCDI/include/SYSGO.h"

#include <array>
#include <cstring>
#include <stdexcept>

/** \brief SoftCDI constructor.
 * \throw std::runtime_error if the given BIOS can't be patched.
 */
SoftCDI::SoftCDI(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
    : CDI("SoftCDI", config, std::move(callbacks), std::move(disc))
    , m_ram0(RAM_BANK_SIZE, 0)
    , m_ram1(RAM_BANK_SIZE, 0)
    , m_bios(std::move(bios))
    // , m_nvramMaxAddress(config.has32KBNVRAM ? 0x330000 : 0x324000)
{
    m_slave = std::make_unique<HLE::IKAT>(*this, config.PAL, 0x310000, PointingDevice::Class::Maneuvering);
    // if(config.has32KBNVRAM)
    //     m_timekeeper = std::make_unique<DS1216>(*this, nvram, config.initialTime);
    // else
    m_timekeeper = std::make_unique<M48T08>(*this, nvram, config.initialTime);
    Reset(true);

    // if(!m_bios.ReplaceModule({NVDRV, NVDRV_len}))
    //     throw std::runtime_error("Failed to patch nvdrv");

    if(!m_bios.ReplaceModule({VIDEO, VIDEO_len}))
        throw std::runtime_error("Failed to patch video");

    // if(!m_bios.ReplaceModule({SYSGO, SYSGO_len}))
    //     throw std::runtime_error("Failed to patch sysgo");

    // Load the reset vector data.
    memcpy(m_ram0.data(), &m_bios[0], 8);
    // printf("%p\n", m_ram0.data());
}

SoftCDI::~SoftCDI()
{
    Stop(true);
}

void SoftCDI::Scheduler()
{
    m_isRunning = true;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::nano>> start = std::chrono::steady_clock::now();

    do
    {
        std::pair<size_t, std::optional<SCC68070::Exception>> res = m_cpu.SingleStepException(25);
        if(res.second.has_value()) // custom syscall to SoftCDI.
            m_cpu.PushException(res.second->vector);

        const double ns = res.first * m_cpu.cycleDelay;
        CDI::IncrementTime(ns);

        start += std::chrono::duration<double, std::nano>(res.first * m_speedDelay);
        std::this_thread::sleep_until(start);
    } while(m_loop);

    m_isRunning = false;
}

void SoftCDI::Reset(const bool resetCPU)
{
    if(resetCPU)
        m_cpu.Reset();
}

uint32_t SoftCDI::GetBIOSBaseAddress() const
{
    return 0x400000;
}

uint32_t SoftCDI::GetTotalFrameCount()
{
    return 0;
}

const OS9::BIOS& SoftCDI::GetBIOS() const
{
    return m_bios;
}

std::vector<InternalRegister> SoftCDI::GetVDSCInternalRegisters()
{
    return {};
}

std::vector<InternalRegister> SoftCDI::GetVDSCControlRegisters()
{
    return {};
}

uint32_t SoftCDI::GetRAMSize() const
{
    return 0x100000;
}

RAMBank SoftCDI::GetRAMBank1() const
{
    return {m_ram0, 0};
}

RAMBank SoftCDI::GetRAMBank2() const
{
    return {m_ram1, 0x200000};
}

const Video::Plane& SoftCDI::GetScreen()
{
    static Video::Plane screen{3};
    return screen;
}

const Video::Plane& SoftCDI::GetPlaneA()
{
    static Video::Plane planeA{3};
    return planeA;
}

const Video::Plane& SoftCDI::GetPlaneB()
{
    static Video::Plane planeB{3};
    return planeB;
}

const Video::Plane& SoftCDI::GetBackground()
{
    static Video::Plane background{3};
    return background;
}

const Video::Plane& SoftCDI::GetCursor()
{
    static Video::Plane cursor{3};
    return cursor;
}
