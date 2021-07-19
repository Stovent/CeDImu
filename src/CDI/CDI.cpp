#include "CDI.hpp"
#include "boards/MiniMMC/MiniMMC.hpp"
#include "boards/Mono3/Mono3.hpp"
#include "cores/M48T08/M48T08.hpp"
#include "OS9/BIOS.hpp"

/** \brief Create a new empty CD-i context.
 * \param conf The configuration of the player.
 */
CDI::CDI(const CDIConfig& conf, const Callbacks& calls) : config(conf), callbacks(calls)
{
}

/** \brief Create a new CD-i context.
 * \param vdscBios System BIOS data.
 * \param vdscSize System BIOS data size.
 * \param brd The type of board to use.
 * \param conf The configuration of the player.

 * See description of CDI::LoadBoard.
 * On failure, CDI::board is a nullptr. On success CDI::board is a valid pointer.
 */
CDI::CDI(const void* vdscBios, const uint32_t vdscSize, const void* nvram, Boards brd, const CDIConfig& conf) : config(conf)
{
    LoadBoard(vdscBios, vdscSize, nvram, brd);
}

CDI::~CDI()
{
    disc.Close();
}

/** \brief Loads a new player type.
 * \param vdscBios System BIOS data.
 * \param vdscSize System BIOS data size.
 * \param boardDetect The type of board to use.
 * \return True if successful, false otherwise.
 *
 * On failure, CDI::board is a nullptr. On success CDI::board is a valid pointer.
 * If \p boardDetect is Boards::AutoDetect, then the board type will be guessed from the BIOS data.
 * It may not be accurate so always privilege providing yourself the board type.
 */
bool CDI::LoadBoard(const void* vdscBios, const uint32_t vdscSize, const void* nvram, Boards boardDetect)
{
    Boards brd;
    if(boardDetect == Boards::AutoDetect)
    {
        brd = DetectBoard((uint8_t*)vdscBios, vdscSize);
        config.has32KBNVRAM = Use32KBNVRAM((uint8_t*)vdscBios, vdscSize);
    }
    else
        brd = boardDetect;

    switch(brd)
    {
    case Boards::MiniMMC:
        board = std::make_unique<MiniMMC>(*this, vdscBios, vdscSize, config);
        break;

    case Boards::Mono3:
    case Boards::Mono4:
    case Boards::Roboco:
        board = std::make_unique<Mono3>(*this, vdscBios, vdscSize, (uint8_t*)nvram, config);
        break;

    default:
        LOG(printf("Failed to init board: %d\n", (int)brd);)
        board = nullptr;
    }

    return board != nullptr;
}

Boards CDI::DetectBoard(const uint8_t* vdscBios, const uint32_t vdscSize) const
{
    if(vdscSize == 523264)
        return Boards::MiniMMC;

    const uint8_t id = vdscBios[vdscSize - 4];
    switch(id >> 4 & 0xF)
    {
    case 2: return Boards::MiniMMC;
    case 3: return Boards::Mono1;
    case 4: return Boards::Mono2;
    case 5: return Boards::Roboco;
    case 6: return Boards::Mono3;
    case 7: return Boards::Mono4;
    default: return Boards::Fail;
    }
}

bool CDI::Use32KBNVRAM(const uint8_t* vdscBios, const uint32_t vdscSize) const
{
    OS9::BIOS bios(vdscBios, vdscSize, 0);

    for(const OS9::ModuleHeader& mod : bios.modules)
    {
        if(mod.name == "nvr")
        {
            uint16_t size = (uint16_t)vdscBios[mod.begin + 74] << 8 | vdscBios[mod.begin + 75];
            if(size == 0x1FF8) // 8KB
                return false;
            return true;
        }
    }
    return false;
}
