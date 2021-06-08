#ifndef CDI_HPP
#define CDI_HPP

#include "CDIConfig.hpp"
#include "CDIDisc.hpp"
#include "boards/Board.hpp"

#include <memory>

enum class Boards
{
    AutoDetect,
    MiniMMC,
    Mono3,
};

class CDI
{
public:
    CDIConfig config; /**< Configuration of the CDI context, only read when loading a board. */
    CDIDisc disc; /**< CDI disc, to be loaded independantly from the board. */
    std::unique_ptr<Board> board; /**< The board representing the CDI player model. */

    CDI(const CDI&) = delete;
    explicit CDI(const CDIConfig& conf = defaultConfig);
    CDI(const void* vdscBios, const uint32_t vdscSize, Boards brd, const CDIConfig& conf = defaultConfig);
    ~CDI();

    bool LoadBoard(const void* vdscBios, const uint32_t vdscSize, Boards boardDetect);
};

#endif // CDI_HPP
