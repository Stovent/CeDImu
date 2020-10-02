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

    void LoadBoard(const void* vdscBios, const uint32_t vdscSize, const void* slaveBios, const uint16_t slaveSize);
};

#endif // CDI_HPP
