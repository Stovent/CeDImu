#include "CDI.hpp"

#include "../Boards/Mono3.hpp"

CDI::CDI() : disk()
{
    board = nullptr;
}

CDI::~CDI()
{
    disk.Close();
    delete board;
}

void CDI::LoadBoard(const void* bios, const uint32_t size)
{
    delete board;

    if(size == 523264)
    {

    }
    else
        board = new Mono3(bios, size);
}
