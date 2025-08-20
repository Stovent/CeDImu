#include "Mono3SoftCDI.hpp"
#include "../../SoftCDI/modules.hpp"

#include <vector>

/** \brief Takes a Mono3 BIOS and replaces its modules with SoftCDI's modules.
 * \throw std::invalid_argument if some Mono3 modules can't be replaced.
 */
static OS9::BIOS makeMono3SoftCDIBIOS(OS9::BIOS mono3)
{
    if(!mono3.ReplaceModule(CIAPDRIV))
        throw std::invalid_argument("can't replace CIAPDRIV");

    if(!mono3.ReplaceModule(SYSGO))
        throw std::invalid_argument("can't replace SYSGO");

    return mono3;
}

Mono3SoftCDI::Mono3SoftCDI(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc, std::string_view baseBoardName)
    : CDI{std::string(baseBoardName) + " SoftCDI", config, std::move(callbacks)}
    , Mono3{makeMono3SoftCDIBIOS(bios), nvram, config, std::move(callbacks)}
    , SoftCDIScheduler{std::move(disc)}
{
}

Mono3SoftCDI::~Mono3SoftCDI() noexcept
{
    Stop(true);
}

void Mono3SoftCDI::IncrementTime(double ns)
{
    Mono3::IncrementTime(ns);
    SoftCDIScheduler::LocalIncrementTime(ns);
}

void Mono3SoftCDI::Reset(bool resetCPU)
{
    Mono3::Reset(resetCPU);
    SoftCDIScheduler::LocalReset();
}
