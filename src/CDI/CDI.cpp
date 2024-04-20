#include "CDI.hpp"
#include "boards/Mono3/Mono3.hpp"

/** \brief Creates a new CD-i instance.
 * \param board The type of board to use.
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
std::unique_ptr<CDI> CDI::NewCDI(Boards board, std::span<const uint8_t> systemBios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
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
        return NewMono3(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));

    case Boards::Mono4:
        return NewMono4(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));

    case Boards::Roboco:
        return NewRoboco(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc));

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

/** \brief Creates a new Mono4 player.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewMono4(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<Mono3>(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc), "Mono-IV");
}

/** \brief Creates a new Roboco player.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewRoboco(OS9::BIOS bios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<Mono3>(std::move(bios), nvram, std::move(config), std::move(callbacks), std::move(disc), "Roboco");
}

CDI::CDI(std::string_view boardName, CDIConfig config, Callbacks callbacks, CDIDisc disc)
    : m_boardName(boardName)
    , m_config(std::move(config))
    , m_disc(std::move(disc))
    , m_callbacks(std::move(callbacks))
    , m_cpu(*this, m_config.PAL ? SCC68070::PAL_FREQUENCY : SCC68070::NTSC_FREQUENCY)
    , m_slave()
    , m_timekeeper()
    , m_schedulerThread()
    , m_loop(false)
    , m_isRunning(false)
    , m_speedDelay(m_cpu.cycleDelay)
{
}

/** \brief Destroys the CDI. Stops and wait for the emulation thread to stop if it is running.
 *
 * Derived destructors MUST call `Stop(true);` too.
 */
CDI::~CDI()
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
        if(m_schedulerThread.joinable())
            m_schedulerThread.join();

        this->m_loop = loop;
        if(loop)
            m_schedulerThread = std::thread(&CDI::Scheduler, this);
        else
            Scheduler();
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
    m_loop = false;
    if(m_schedulerThread.joinable())
    {
        if(wait)
            m_schedulerThread.join();
        else
            m_schedulerThread.detach();
    }
}

/** \brief Set the CPU emulated speed.
 *
 * \param speed The speed multiplier based on the clock frequency used in the constructor.
 *
 * This method only changes the emulation speed, not the clock frequency.
 * A multiplier of 2 will make the CPU runs twice as fast, the GPU to run at twice the framerate,
 * the timekeeper to increment twice as fast, etc.
 */
void CDI::SetEmulationSpeed(const double speed)
{
    m_speedDelay = m_cpu.cycleDelay / speed;
}

/** \brief Returns a pointer to the given address.
 */
const uint8_t* CDI::GetPointer(uint32_t addr) const
{
    const RAMBank ram1 = GetRAMBank1();
    if(addr >= ram1.base && addr < ram1.base + ram1.data.size())
        return &ram1.data[addr - ram1.base];

    const RAMBank ram2 = GetRAMBank2();
    if(addr >= ram2.base && addr < ram2.base + ram2.data.size())
        return &ram2.data[addr - ram2.base];

    const OS9::BIOS& bios = GetBIOS();
    const uint32_t base = GetBIOSBaseAddress();
    if(addr >= base && addr < base + bios.GetSize())
        return bios(addr - base);

    return nullptr;
}

void CDI::IncrementTime(const double ns)
{
    m_slave->IncrementTime(ns);
    m_timekeeper->IncrementClock(ns);
}
