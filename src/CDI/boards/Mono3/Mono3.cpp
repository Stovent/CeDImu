#include "Mono3.hpp"
#include "../../common/Callbacks.hpp"
#include "../../HLE/IKAT/IKAT.hpp"
#include "../../cores/DS1216/DS1216.hpp"
#include "../../cores/M48T08/M48T08.hpp"

Mono3::Mono3(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc, std::string_view boardName)
    : CDI(boardName, config, std::move(callbacks), std::move(disc))
    , m_mcd212(*this, std::move(bios), config.PAL)
    , m_ciap(*this)
    , m_nvramMaxAddress(config.has32KBNVRAM ? 0x330000 : 0x324000)
{
    m_slave = std::make_unique<HLE::IKAT>(*this, config.PAL, 0x310000, PointingDevice::Class::Maneuvering);
    if(config.has32KBNVRAM)
        m_timekeeper = std::make_unique<DS1216>(*this, nvram, config.initialTime);
    else
        m_timekeeper = std::make_unique<M48T08>(*this, nvram, config.initialTime);
    Reset(true);
}

Mono3::~Mono3() noexcept
{
    Stop(true);
}

void Mono3::Scheduler(const std::stop_token stopToken)
{
    m_isRunning = true;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::nano>> start = std::chrono::steady_clock::now();

    do
    {
        const SCC68070::InterpreterResult res = m_cpu.SingleStep(25);
        const size_t cycles = res.first;

        const double ns = cycles * m_cpu.cycleDelay;
        CDI::IncrementTime(ns);
        m_mcd212.IncrementTime(ns);
        m_ciap.IncrementTime(ns);

        start += std::chrono::duration<double, std::nano>(cycles * m_speedDelay);
        std::this_thread::sleep_until(start);
    } while(!stopToken.stop_requested());

    m_isRunning = false;
}

void Mono3::Reset(const bool resetCPU)
{
    m_mcd212.Reset();
    if(resetCPU)
        m_cpu.Reset();
}

uint32_t Mono3::GetTotalFrameCount()
{
    return m_mcd212.m_totalFrameCount;
}

const OS9::BIOS& Mono3::GetBIOS() const
{
    return m_mcd212.m_bios;
}

uint32_t Mono3::GetBIOSBaseAddress() const
{
    return 0x400000;
}

std::vector<InternalRegister> Mono3::GetVDSCInternalRegisters()
{
    return m_mcd212.GetInternalRegisters();
}

std::vector<InternalRegister> Mono3::GetVDSCControlRegisters()
{
    return m_mcd212.GetControlRegisters();
}

uint32_t Mono3::GetRAMSize() const
{
    return 0x100000;
}

RAMBank Mono3::GetRAMBank1() const
{
    return m_mcd212.GetRAMBank1();
}

RAMBank Mono3::GetRAMBank2() const
{
    return m_mcd212.GetRAMBank2();
}

const Video::Plane& Mono3::GetScreen()
{
    return m_mcd212.GetScreen();
}

const Video::Plane& Mono3::GetPlaneA()
{
    return m_mcd212.GetPlaneA();
}

const Video::Plane& Mono3::GetPlaneB()
{
    return m_mcd212.GetPlaneB();
}

const Video::Plane& Mono3::GetBackground()
{
    return m_mcd212.GetBackground();
}

const Video::Plane& Mono3::GetCursor()
{
    return m_mcd212.GetCursor();
}
