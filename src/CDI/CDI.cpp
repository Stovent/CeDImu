#include "CDI.hpp"
#include "boards/MiniMMC/MiniMMC.hpp"
#include "boards/Mono3/Mono3.hpp"

CDI::CDI()
{
}

CDI::~CDI()
{
    disc.Close();
}

void CDI::LoadBoard(const void* vdscBios, const uint32_t vdscSize, const bool initNVRAMClock, const bool PAL)
{
    if(vdscSize == 523264)
        board = std::make_unique<MiniMMC>(vdscBios, vdscSize, initNVRAMClock);
    else
        board = std::make_unique<Mono3>(vdscBios, vdscSize, initNVRAMClock, PAL);
}
