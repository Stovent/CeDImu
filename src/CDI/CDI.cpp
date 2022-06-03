#include "CDI.hpp"
#include "boards/MiniMMC/MiniMMC.hpp"
#include "boards/Mono2/Mono2.hpp"
#include "boards/Mono3/Mono3.hpp"
#include "cores/M48T08/M48T08.hpp"
#include "OS9/BIOS.hpp"

/** \brief Create a new empty CD-i context.
 * \param conf The configuration of the player.
 * \param calls The user callbacks.
 */
CDI::CDI(const CDIConfig& conf, const Callbacks& calls)
    : config(conf)
    , disc()
    , board()
    , callbacks(calls)
{
}

/** \brief Create a new CD-i context.
 * \param vdscBios System BIOS data.
 * \param vdscSize System BIOS data size.
 * \param nvram NVRAM data from saved file.
 * \param brd The type of board to use.
 * \param conf The configuration of the player.
 * \param calls The user callbacks.
 *
 * See description of CDI::LoadBoard.
 * On failure, CDI::board is a nullptr. On success CDI::board is a valid pointer.
 */
CDI::CDI(const void* vdscBios, const uint32_t vdscSize, const void* nvram, Boards brd, const CDIConfig& conf, const Callbacks& calls)
    : config(conf)
    , disc()
    , board()
    , callbacks(calls)
{
    LoadBoard(vdscBios, vdscSize, nvram, brd);
}

CDI::~CDI()
{
    UnloadBoard();
    disc.Close();
}

/** \brief Loads a new player type.
 * \param vdscBios System BIOS data.
 * \param vdscSize System BIOS data size.
 * \param nvram NVRAM data from saved file.
 * \param boardDetect The type of board to use.
 * \return True if successful, false otherwise.
 *
 * On failure, CDI::board is a nullptr. On success CDI::board is a valid pointer.
 * If \p boardDetect is Boards::AutoDetect, then the board type will be guessed from the BIOS data.
 * It may not be accurate so always privilege providing yourself the board type.
 *
 * User needs to ensure the NVRAM data size corresponds to the NVRAM size in the config (or auto-detected).
 */
bool CDI::LoadBoard(const void* vdscBios, const uint32_t vdscSize, const void* nvram, Boards boardDetect)
{
    UnloadBoard();
    const OS9::BIOS bios(vdscBios, vdscSize, 0);

    Boards brd;
    if(boardDetect == Boards::AutoDetect)
    {
        brd = bios.GetBoardType();
        config.has32KBNVRAM = !bios.Has8KBNVRAM();
    }
    else
        brd = boardDetect;

    switch(brd)
    {
//    case Boards::MiniMMC:
//        board = std::make_unique<MiniMMC>(*this, vdscBios, vdscSize, config);
//        break;

//    case Boards::Mono2:
//        board = std::make_unique<Mono2>(*this, vdscBios, vdscSize, (uint8_t*)nvram, config);
//        break;

    case Boards::Mono3:
    case Boards::Mono4:
    case Boards::Roboco:
        board = std::make_unique<Mono3>(*this, vdscBios, vdscSize, (uint8_t*)nvram, config);
        break;

    default:
        LOG(printf("Failed to init board: %d\n", (int)brd);)
        board = nullptr;
    }

    return (bool)board;
}

void CDI::UnloadBoard()
{
    if(board)
        board->cpu.Stop(true);
    board.reset();
}
