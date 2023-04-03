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
    if(board == Boards::AutoDetect)
    {
        const OS9::BIOS bios(systemBios, 0);
        board = bios.GetBoardType();
        config.has32KBNVRAM = !bios.Has8KBNVRAM();
    }

    switch(board)
    {
    case Boards::Mono3:
        return NewMono3(systemBios, nvram, std::move(config), std::move(callbacks), std::move(disc));

    case Boards::Mono4:
        return NewMono4(systemBios, nvram, std::move(config), std::move(callbacks), std::move(disc));

    case Boards::Roboco:
        return NewRoboco(systemBios, nvram, std::move(config), std::move(callbacks), std::move(disc));

    default:
        return nullptr;
    }
}

/** \brief Creates a new Mono3 player.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewMono3(std::span<const uint8_t> systemBios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<Mono3>(systemBios, nvram, std::move(config), std::move(callbacks), std::move(disc));
}

/** \brief Creates a new Mono4 player.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewMono4(std::span<const uint8_t> systemBios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<Mono3>(systemBios, nvram, std::move(config), std::move(callbacks), std::move(disc), "Mono-IV");
}

/** \brief Creates a new Roboco player.
 * See CDI::NewCDI for a description of the parameters.
 */
std::unique_ptr<CDI> CDI::NewRoboco(std::span<const uint8_t> systemBios, std::span<const uint8_t> nvram, CDIConfig config, Callbacks callbacks, CDIDisc disc)
{
    return std::make_unique<Mono3>(systemBios, nvram, std::move(config), std::move(callbacks), std::move(disc), "Roboco");
}

CDI::CDI(std::string_view boardName, CDIConfig config, Callbacks callbacks, CDIDisc disc)
    : m_boardName(boardName)
    , m_config(std::move(config))
    , m_disc(std::move(disc))
    , m_callbacks(std::move(callbacks))
    , m_cpu(*this, m_config.PAL ? SCC68070::PAL_FREQUENCY : SCC68070::NTSC_FREQUENCY)
    , m_slave()
    , m_timekeeper()
{
}

/** \brief Destroys the CDI. Stops and wait for the emulation thread to stop if it is running.
 */
CDI::~CDI()
{
    m_cpu.Stop(true);
}

/** \brief Returns a pointer to the given address.
 */
const uint8_t* CDI::GetPointer(uint32_t addr) const
{
    const RAMBank ram1 = GetRAMBank1();
    const RAMBank ram2 = GetRAMBank2();
    const OS9::BIOS& bios = GetBIOS();

    if(addr >= ram1.base && addr < ram1.base + ram1.data.size())
        return &ram1.data[addr - ram1.base];

    if(addr >= ram2.base && addr < ram2.base + ram2.data.size())
        return &ram2.data[addr - ram2.base];

    if(addr >= bios.m_base && addr < bios.m_base + bios.m_size)
        return bios(addr - bios.m_base);

    return nullptr;
}

void CDI::IncrementTime(double ns)
{
    m_slave->IncrementTime(ns);
    m_timekeeper->IncrementClock(ns);
}
