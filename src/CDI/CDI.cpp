#include "CDI.hpp"
#include "../Boards/MiniMMC/MiniMMC.hpp"
#include "../Boards/Mono3/Mono3.hpp"

CDI::CDI() : disc()
{
    board = nullptr;
}

CDI::~CDI()
{
    disc.Close();
    delete board;
}

void CDI::LoadBoard(const void* vdscBios, const uint32_t vdscSize, const void* slaveBios, const uint16_t slaveSize)
{
    delete board;

    if(vdscSize == 523264)
        board = new MiniMMC(vdscBios, vdscSize);
    else
        board = new Mono3(vdscBios, vdscSize, slaveBios, slaveSize);
}
