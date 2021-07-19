#ifndef CDI_HPP
#define CDI_HPP

#include "CDIConfig.hpp"
#include "CDIDisc.hpp"
#include "boards/Board.hpp"
#include "common/Callbacks.hpp"

#include <memory>

enum class Boards
{
    AutoDetect,
    MiniMMC,
    Mono1,
    Mono2,
    Mono3,
    Mono4,
    Roboco,
    Fail,
};

class CDI
{
public:
    CDIConfig config; /**< Configuration of the CDI context, only read when loading a board. */
    CDIDisc disc; /**< CDI disc, to be loaded independantly from the board. */
    std::unique_ptr<Board> board; /**< The board representing the CDI player model. */
    Callbacks callbacks; /**< The user callbacks. */

    CDI(const CDI&) = delete;
    explicit CDI(const CDIConfig& conf = defaultConfig, const Callbacks& calls = Callbacks());
    CDI(const void* vdscBios, const uint32_t vdscSize, const void* nvram, Boards brd, const CDIConfig& conf = defaultConfig);
    ~CDI();

    bool LoadBoard(const void* vdscBios, const uint32_t vdscSize, const void* nvram, Boards boardDetect);
    Boards DetectBoard(const uint8_t* vdscBios, const uint32_t vdscSize) const;
    bool Use32KBNVRAM(const uint8_t* vdscBios, const uint32_t vdscSize) const;
};

#endif // CDI_HPP
