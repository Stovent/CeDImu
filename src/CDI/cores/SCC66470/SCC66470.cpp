#include "SCC66470.hpp"
#include "../../boards/Board.hpp"
#include "../../common/Callbacks.hpp"

#include <cstring>

SCC66470::SCC66470(CDI& idc, const bool ismaster, const void* bios, const uint32_t size)
    : cdi(idc)
    , BIOS(bios, size, 0x180000)
    , totalFrameCount(0)
    , isMaster(ismaster)
    , memory(0x100000, 0)
    , memorySwapCount(0)
    , lineNumber(0)
    , internalRegisters{}
    , registerCSR(0)
    , registerB(0)
    , screen(Video::Plane::RGB_SIZE)
    , planeA()
    , planeB()
    , cursorPlane(Video::Plane::CURSOR_ARGB_SIZE, 16, 16)
    , backgroundPlane(Video::Plane::MAX_HEIGHT * 4, 1, Video::Plane::MAX_HEIGHT)
{
    std::fill(internalRegisters.begin(), internalRegisters.end(), 0);

    memorySwapCount = 0;
}

void SCC66470::Reset()
{
    MemorySwap();
}

void SCC66470::IncrementTime(const double)
{
}

void SCC66470::MemorySwap()
{
    memorySwapCount = 0;
}

RAMBank SCC66470::GetRAMBank1() const
{
    return {{memory.data(), 0x80000}, 0};
}

RAMBank SCC66470::GetRAMBank2() const
{
    return {{&memory[0x80000], 0x80000}, 0x80000};
}

const Video::Plane& SCC66470::GetScreen() const
{
    return screen;
}

const Video::Plane& SCC66470::GetPlaneA() const
{
    return planeA;
}

const Video::Plane& SCC66470::GetPlaneB() const
{
    return planeB;
}

const Video::Plane& SCC66470::GetBackground() const
{
    return backgroundPlane;
}

const Video::Plane& SCC66470::GetCursor() const
{
    return cursorPlane;
}
