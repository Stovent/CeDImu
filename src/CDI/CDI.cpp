#include "CDI.hpp"
#include "boards/Mono3/Mono3.hpp"
#include "boards/Mono3SoftCDI/Mono3SoftCDI.hpp"
#include "boards/SoftCDI/SoftCDI.hpp"

#include <version>

/** \brief Creates a new CD-i instance.
 * \param board The type of board to use.
 * \param useSoftCDI false to use the regular LLE emulator, true to use some SoftCDI modules.
 * \param systemBios System BIOS data.
 * \param nvram The initial state of the NVRAM, or an empty span to use a clean NVRAM.
 * \param config The player configuration.
 * \param callbacks The user callbacks to use.
 * \param disc The disc to load (optional).
 * \return nullptr if it failed to initialize the new CDI.
 *
 * If \p board is Boards::AutoDetect, then the board type will be guessed from the BIOS data.
 * It may not be accurate so when possible, consider providing yourself the board type.
 *
 * If you choose not to load a disc, you can load it later by accessing CDI::m_disc.
 *
 * User needs to ensure the NVRAM data size corresponds to the NVRAM size in the config (or auto-detected).
 */
std::unique_ptr<CDI> CDI::NewCDI(Boards board, bool useSoftCDI, std::span<const uint8_t> systemBios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    const OS9::BIOS bios(systemBios);

    if(board == Boards::AutoDetect)
    {
        board = bios.GetBoardType();
        config.has32KBNVRAM = !bios.Has8KBNVRAM();
    }

    switch(board)
    {
    case Boards::Mono3:
        if(useSoftCDI)
            return NewMono3SoftCDI(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));
        else
            return NewMono3(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));

    case Boards::Mono4:
        if(useSoftCDI)
            return NewMono4SoftCDI(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));
        else
            return NewMono4(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));

    case Boards::Roboco:
        if(useSoftCDI)
            return NewRobocoSoftCDI(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));
        else
            return NewRoboco(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));

    case Boards::SoftCDI:
        return NewSoftCDI(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));

    default:
        return nullptr;
    }
}

/** \brief Creates a new Mono3 player.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewMono3(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<Mono3>(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));
}

/** \brief Creates a new Mono3 player with SoftCDI modules.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewMono3SoftCDI(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<Mono3SoftCDI>(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));
}

/** \brief Creates a new Mono4 player.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewMono4(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<Mono3>(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc), "Mono-IV");
}

/** \brief Creates a new Mono4 player with SoftCDI modules.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewMono4SoftCDI(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<Mono3SoftCDI>(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc), "Mono-IV");
}

/** \brief Creates a new Roboco player.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewRoboco(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<Mono3>(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc), "Roboco");
}

/** \brief Creates a new Roboco player with SoftCDI modules.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewRobocoSoftCDI(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<Mono3SoftCDI>(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc), "Roboco");
}

/** \brief Creates a new SoftCDI player.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewSoftCDI(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<SoftCDI>(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));
}

CDI::CDI(std::string_view boardName, CDIConfig config, Callbacks callbacks, CDIDisc disc)
    : m_boardName(boardName)
    , m_config(std::move(config))
    , m_callbacks(std::move(callbacks))
    , m_cpu(*this, m_config.PAL ? SCC68070::PAL_FREQUENCY : SCC68070::NTSC_FREQUENCY)
    , m_disc(std::move(disc))
{
}

/** \brief Destroys the CDI. Stops and wait for the emulation thread to stop if it is running.
 *
 * Derived destructors MUST call `Stop(true);` too.
 */
CDI::~CDI() noexcept
{
    Stop(true);
}

/** \brief Start emulation.
 *
 * \param loop If true, will run indefinitely as a thread. If false, will execute a single scheduler cycle.
 *
 * If loop = true, executes indefinitely in a thread (non-blocking).
 * If loop = false, executes a single scheduler cycle and returns when it is executed (blocking).
 */
void CDI::Run(const bool loop)
{
    if(!m_isRunning)
    {
        if(loop)
            m_schedulerThread = std::jthread(std::bind_front(&CDI::Scheduler, this));
        else
        {
            std::stop_source stopSource{};
            stopSource.request_stop();
            Scheduler(stopSource.get_token());
        }
    }
}

/** \brief Stop emulation.
 *
 * \param wait If true, will wait for the thread to join. If false, detach the thread while it stops.
 *
 * If this is invoked during a callback, false must be sent to prevent any dead lock.
 */
void CDI::Stop(const bool wait)
{
    m_schedulerThread.request_stop();
    if(m_schedulerThread.joinable())
    {
        if(wait)
            m_schedulerThread.join();
        else
            m_schedulerThread.detach();
    }
}

std::span<const uint8_t> CDI::GetPointer(const uint32_t addr) const noexcept
{
    const RAMBank ram1 = GetRAMBank1();
    if(addr >= ram1.base && addr < ram1.base + ram1.data.size())
#if __cpp_lib_ranges_as_const >= 202207L
        return {ram1.data.cbegin() + (addr - ram1.base), ram1.data.cend()};
#else
        return {ram1.data.begin() + (addr - ram1.base), ram1.data.end()};
#endif
    // LLVM doesn't implement it yet.

    const RAMBank ram2 = GetRAMBank2();
    if(addr >= ram2.base && addr < ram2.base + ram2.data.size())
#if __cpp_lib_ranges_as_const >= 202207L
        return {ram2.data.cbegin() + (addr - ram2.base), ram2.data.cend()};
#else
        return {ram2.data.begin() + (addr - ram2.base), ram2.data.end()};
#endif

    const OS9::BIOS& bios = GetBIOS();
    const uint32_t base = GetBIOSBaseAddress();
    if(addr >= base && addr < base + bios.GetSize())
        return {bios.CBegin() + (addr - base), bios.CEnd()};

    return {};
}

// std::span<uint8_t> CDI::GetPointer(const uint32_t addr) noexcept
// {
//     const RAMBank ram1 = GetRAMBank1();
//     if(addr >= ram1.base && addr < ram1.base + ram1.data.size())
//         return ram1.data;
//
//     const RAMBank ram2 = GetRAMBank2();
//     if(addr >= ram2.base && addr < ram2.base + ram2.data.size())
//         return ram2.data;
//
//     return {};
// }

CDIDisc& CDI::GetDisc() noexcept
{
    return m_disc;
}

void CDI::Scheduler(const std::stop_token stopToken)
{
    m_isRunning = true;

    do
    {
        const SCC68070::InterpreterResult res = m_cpu.SingleStep(25);
        const size_t cycles = res.first;

        const double ns = cycles * m_cpu.cycleDelay;
        IncrementTime(ns);
    } while(!stopToken.stop_requested());

    m_isRunning = false;
}

void CDI::IncrementTime(const double ns)
{
    m_slave->IncrementTime(ns);
    m_timekeeper->IncrementClock(ns);
}

void CDI::Reset(bool resetCPU)
{
    if(resetCPU)
        m_cpu.Reset();
    // TODO: reset slave and IRTC too?
}
