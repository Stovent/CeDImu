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
};

#endif // CDI_HPP
