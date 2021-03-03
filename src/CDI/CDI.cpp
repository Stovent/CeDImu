#include "CDI.hpp"
#include "boards/MiniMMC/MiniMMC.hpp"
#include "boards/Mono3/Mono3.hpp"

CDI::CDI() : disc()
{
    board = nullptr;
}

CDI::~CDI()
{
    disc.Close();
    delete board;
}

void CDI::LoadBoard(const void* vdscBios, const uint32_t vdscSize, const bool initNVRAMClock)
{
    delete board;

    if(vdscSize == 523264)
        board = new MiniMMC(vdscBios, vdscSize, initNVRAMClock);
    else
        board = new Mono3(vdscBios, vdscSize, initNVRAMClock);
}
