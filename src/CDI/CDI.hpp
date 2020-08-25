#ifndef CDI_HPP
#define CDI_HPP

class CDI;

#include "CDIDisk.hpp"
#include "../Boards/Board.hpp"

class CDI
{
public:
    CDIDisk disk;
    Board* board;

    CDI();
    ~CDI();

    void LoadBoard(const void* bios, const uint32_t size);
};

#endif // CDI_HPP
