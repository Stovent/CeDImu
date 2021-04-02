#ifndef CDI_HPP
#define CDI_HPP

#include "CDIDisc.hpp"
#include "boards/Board.hpp"

#include <memory>

class CDI
{
public:
    CDIDisc disc;
    std::unique_ptr<Board> board;

    CDI();
    ~CDI();

    void LoadBoard(const void* vdscBios, const uint32_t vdscSize, const bool initNVRAMClock, const bool PAL);
};

#endif // CDI_HPP
