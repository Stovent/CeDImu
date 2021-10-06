#include "SCC66470.hpp"
#include "../../boards/Board.hpp"
#include "../../common/Callbacks.hpp"

#include <cstring>

SCC66470::SCC66470(CDI& idc, const bool ismaster, const void* bios, const uint32_t size) :
    VDSC(idc, bios, size, 0x180000),
    memory(0x100000, 0),
    isMaster(ismaster)
{
    std::fill(internalRegisters.begin(), internalRegisters.end(), 0);

    memorySwapCount = 0;
}

SCC66470::~SCC66470()
{
}

void SCC66470::Reset()
{
    MemorySwap();
}

void SCC66470::IncrementTime(const double ns)
{
}

void SCC66470::MemorySwap()
{
    memorySwapCount = 0;
}

void SCC66470::PutDataInMemory(const void* s, unsigned int size, unsigned int position)
{
    memcpy(&memory[position], s, size);
}

void SCC66470::WriteToBIOSArea(const void* s, unsigned int size, unsigned int position)
{
    PutDataInMemory(s, size, position + 0x180000);
}

RAMBank SCC66470::GetRAMBank1() const
{
    return {memory.data(), 0, 0x80000};
}

RAMBank SCC66470::GetRAMBank2() const
{
    return {&memory[0x80000], 0x80000, 0x80000};
}

const Plane& SCC66470::GetScreen() const
{
    return screen;
}

const Plane& SCC66470::GetPlaneA() const
{
    return planeA;
}

const Plane& SCC66470::GetPlaneB() const
{
    return planeB;
}

const Plane& SCC66470::GetBackground() const
{
    return backgroundPlane;
}

const Plane& SCC66470::GetCursor() const
{
    return cursorPlane;
}

#undef   SET_DA_BIT
#undef UNSET_DA_BIT
