#ifndef CDI_HPP
#define CDI_HPP

#include "CDIDisc.hpp"
#include "boards/Board.hpp"

class CDI
{
public:
    CDIDisc disc;
    Board* board;

    CDI();
    ~CDI();

    void LoadBoard(const void* vdscBios, const uint32_t vdscSize, const bool initNVRAMClock);
};

#endif // CDI_HPP
