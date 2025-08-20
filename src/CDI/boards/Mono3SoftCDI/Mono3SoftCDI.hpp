#ifndef CDI_BOARDS_MONO3SOFTCDI_MONO3SOFTCDI_HPP
#define CDI_BOARDS_MONO3SOFTCDI_MONO3SOFTCDI_HPP

#include "../Mono3/Mono3.hpp"
#include "../SoftCDIScheduler/SoftCDIScheduler.hpp"

#include <span>

class Mono3SoftCDI : virtual public Mono3, virtual public SoftCDIScheduler
{
public:
    Mono3SoftCDI(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc = CDIDisc(), std::string_view baseBoardName = "Mono-III");
    virtual ~Mono3SoftCDI() noexcept;

protected:
    virtual void Scheduler(std::stop_token stopToken) override { SoftCDIScheduler::Scheduler(stopToken); }
    virtual void IncrementTime(double ns) override;

    virtual void Reset(bool resetCPU) override;
};

#endif // CDI_BOARDS_MONO3SOFTCDI_MONO3SOFTCDI_HPP
