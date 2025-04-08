#include "SoftCDI.hpp"
#include "../../common/Callbacks.hpp"
#include "../../HLE/IKAT/IKAT.hpp"
#include "../../cores/DS1216/DS1216.hpp"
#include "../../cores/M48T08/M48T08.hpp"

#include "../../SoftCDI/include/CIAPDRIV.h"
#include "../../SoftCDI/include/CSD_450.h"
#include "../../SoftCDI/include/NVDRV.h"
#include "../../SoftCDI/include/VIDEO.h"
#include "../../SoftCDI/include/SYSGO.h"

#include <array>
#include <cinttypes>
#include <cstring>
#include <stdexcept>

/** \brief Generates a SoftCDI BIOS with core modules taken from the given Mono3 bios.
 * \return The SoftCDI BIOS.
 * \throw std::invalid_argument if some Mono3 modules are incorrect.
 * \throw std::length_error when the resulting BIOS is bigger than SoftCDI::BIOS_SIZE.
 */
static OS9::BIOS makeSoftcdiBiosFromMono3(const OS9::BIOS& mono3)
{
    const OS9::ModuleHeader& kernel = mono3.GetModules().front();
    if(kernel.name != "kernel")
        throw std::invalid_argument("First Mono3 module must be the kernel");

    // Copy the boot code and the kernel module.
    std::vector<uint8_t> bios(mono3.CBegin(), mono3.CBegin() + kernel.end);

    // Copy the core modules.
    for(const OS9::ModuleHeader& module : mono3.GetModules())
    {
        if(module.name == "cio" ||
           module.name == "cd" ||
           module.name == "init" ||
           module.name == "t2" || // Serial port
           module.name == "u68070" || // device driver for t2
           module.name == "scf" || // File manager for t2
           module.name == "sgstom" || // Clock module
           module.name == "tim070" || // timer device descriptor
           module.name == "tim070driv" || // timer device driver
           module.name == "nvr" || // NVRAM descriptor
           module.name == "nrf" || // NVRAM file manager
           module.name == "nvdrv" || // NVRAM device driver
           module.name == "csdinit" ||
           module.name == "ucm" ||
        //    module.name == "csd_450" ||
           module.name == "cdfm")
        {
            bios.insert(bios.end(), mono3.CBegin() + module.begin, mono3.CBegin() + module.end);
        }

        // if(module.name == "dummy")
        // {
        //     break; // dummy needs to be the last module.
        // }
    }

    // Add custom mdules.
    // bios.append_range(std::span{SYSGO, SYSGO_len});
    bios.insert(bios.end(), CIAPDRIV, &CIAPDRIV[CIAPDRIV_len]);
    bios.insert(bios.end(), CSD_450, &CSD_450[CSD_450_len]);
    bios.insert(bios.end(), SYSGO, &SYSGO[SYSGO_len]);

    /*// Add dummy module.
    const OS9::ModuleHeader& dummy = mono3.GetModules().back();
    if(dummy.name != "dummy")
        throw std::invalid_argument("Last Mono3 module must be dummy");
    bios.insert(bios.end(), mono3.CBegin() + dummy.begin, mono3.CBegin() + dummy.end);*/

    if(bios.size() >= SoftCDI::BIOS_SIZE)
        throw std::length_error("SoftCDI BIOS must not exceed 0x08'0000 bytes");

    bios.resize(SoftCDI::BIOS_SIZE); // Mono3 boot code expects the BIOS size to be 0x8'0000 when searching ROMed module.

    return OS9::BIOS({bios.begin(), bios.end()});
}

/** \brief SoftCDI constructor.
 * \throw std::runtime_error if the given BIOS can't be patched.
 */
SoftCDI::SoftCDI(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
    : CDI("SoftCDI", config, std::move(callbacks), std::move(disc))
    , m_ram0(RAM_BANK_SIZE, 0)
    , m_ram1(RAM_BANK_SIZE, 0)
    , m_bios(makeSoftcdiBiosFromMono3(bios))
    // , m_nvramMaxAddress(config.has32KBNVRAM ? 0x330000 : 0x324000)
{
    m_slave = std::make_unique<HLE::IKAT>(*this, config.PAL, 0x310000, PointingDevice::Class::Maneuvering);
    // if(config.has32KBNVRAM)
    //     m_timekeeper = std::make_unique<DS1216>(*this, nvram, config.initialTime);
    // else
    m_timekeeper = std::make_unique<M48T08>(*this, nvram, config.initialTime);
    Reset(true);

    // Load the reset vector data.
    memcpy(m_ram0.data(), &m_bios[0], 8);
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
        const SCC68070::InterpreterResult res = m_cpu.SingleStepException(25);

        const double ns = res.first * m_cpu.cycleDelay;
        CDI::IncrementTime(ns);

        start += std::chrono::duration<double, std::nano>(res.first * m_speedDelay);
        std::this_thread::sleep_until(start);

        if(std::holds_alternative<SCC68070::Exception>(res.second))
        {
            const SCC68070::Exception ex = std::get<SCC68070::Exception>(res.second);

            if(ex.vector == SCC68070::Trap0Instruction && ex.data > SystemCalls::_Min) // SoftCDI syscall.
            {
                const uint16_t syscall = ex.data;

                m_cpu.GetNextWord(); // Skip syscall ID when returning.
                printf("softcdi syscall %" PRIX16 "\n", syscall);
                DispatchSystemCall(syscall);
            }
            else // CPU exception/OS-9 syscall.
            {
                m_cpu.PushException(ex);
            }
        }
        else if(std::holds_alternative<SCC68070::Breakpoint>(res.second))
        {
            break;
        }
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
    return BIOSBegin;
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
    return {m_ram0, RAM0Begin};
}

RAMBank SoftCDI::GetRAMBank2() const
{
    return {m_ram1, RAM1Begin};
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
