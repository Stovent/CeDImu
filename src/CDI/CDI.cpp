#include "CDI.hpp"
#include "boards/MiniMMC/MiniMMC.hpp"
#include "boards/Mono3/Mono3.hpp"

constexpr CDIConfig defaultConfig = {
    .PAL = true,
    .initialTime = M48T08::defaultTime,
};

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
CDI::CDI(const void* vdscBios, const uint32_t vdscSize, Boards brd, const CDIConfig& conf) : config(conf)
{
    LoadBoard(vdscBios, vdscSize, brd);
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
bool CDI::LoadBoard(const void* vdscBios, const uint32_t vdscSize, Boards boardDetect)
{
    Boards brd;
    if(boardDetect == Boards::AutoDetect)
    {
        if(vdscSize == 523264)
            brd = Boards::MiniMMC;
        else
            brd = Boards::Mono3;
    }
    else
        brd = boardDetect;

    switch(brd)
    {
    case Boards::MiniMMC:
        board = std::make_unique<MiniMMC>(*this, vdscBios, vdscSize, config);
        break;

    case Boards::Mono3:
        board = std::make_unique<Mono3>(*this, vdscBios, vdscSize, config);
        break;

    default:
        board = nullptr;
    }

    return board != nullptr;
}
