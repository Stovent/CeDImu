#include "CDI.hpp"
#include "boards/Mono3/Mono3.hpp"

#include <functional>

/** \brief Creates a new CD-i instance.
 * \param board The type of board to use.
 * \param systemBios System BIOS data (in big endian format).
 * \param nvram The initial state of the NVRAM, or an empty span to use a clean NVRAM.
 * \param config The player configuration.
 * \param callbacks The user callbacks to use.
 * \param disc The disc to load (optional).
 * \return nullptr if the BIOS is not supported or if it failed to auto detect the CDI BIOS type.
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
{
}

/** \brief Destroys the CDI. Stops and wait for the emulation thread to stop if it is running.
 */
CDI::~CDI() noexcept
{
    m_cpu.Stop(true);
}

/** \brief Returns a pointer to the given address.
 */
const uint8_t* CDI::GetPointer(const uint32_t addr) const
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
        return &bios[addr - base];

    return nullptr;
}

OS9::EmulatedMemoryAccess CDI::GetEmulatedMemoryAccessCallbacks() const
{
    return OS9::EmulatedMemoryAccess {
        .GetByte = std::bind(&CDI::PeekByte, this, std::placeholders::_1),
        .GetWord = std::bind(&CDI::PeekWord, this, std::placeholders::_1),
        .SetByte = nullptr,
        .SetWord = nullptr,
    };
}

/** \brief Returns a pointer to the kernel for inspection, or nullptr if the system globals is not valid yet. */
const OS9::Kernel* CDI::GetKernel() const
{
    if(m_kernel.IsValid())
        return &m_kernel;
    else
        return nullptr;
}

/** \brief Returns the name of the module that contains the given address instruction.
 * This searches in the system globals module list first. If it is not valid yet it searches in the ROMed modules only.
 */
std::string CDI::GetModuleNameAt(const uint32_t addr) const
{
    const OS9::Kernel* kernel = GetKernel();
    if(kernel != nullptr)
        return kernel->GetModuleNameAt(addr);

    return GetBIOS().GetModuleNameAt(addr - GetBIOSBaseAddress());
}

void CDI::IncrementTime(const double ns)
{
    m_slave->IncrementTime(ns);
    m_timekeeper->IncrementClock(ns);
}
