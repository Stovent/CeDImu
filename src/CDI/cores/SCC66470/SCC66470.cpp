#include "SCC66470.hpp"
#include "../../boards/Board.hpp"
#include "../../common/utils.hpp"

#include <cstdio>
#include <cstring>

SCC66470::SCC66470(Board& board, const bool ismaster) : VDSC(board), isMaster(ismaster)
{
    memorySwapCount = 0;
    stopOnNextFrame = false;
    allocatedMemory = 0x200000;

    OPEN_LOG(out_dram, isMaster ? "SCC66470_master_DRAM.txt" : "SCC66470_slave_DRAM.txt")

    memory = new uint8_t[allocatedMemory];
    memset(memory, 0, allocatedMemory);
    memset(internalRegisters, 0, 0x20 * sizeof *internalRegisters);
}

SCC66470::~SCC66470()
{
    CLOSE_LOG(out_dram)
    delete[] memory;
}

void SCC66470::Reset()
{
    MemorySwap();
}

void SCC66470::MemorySwap()
{
    memorySwapCount = 0;
}

bool SCC66470::LoadBIOS(const void* bios, uint32_t size) // only CD-I 205, it should be 523 264 bytes long
{
    if(size > 0x7FC00)
        size = 0x7FC00;

    memcpy(&memory[0x180000], bios, size);

    return biosLoaded = true;
}

void SCC66470::PutDataInMemory(const void* s, unsigned int size, unsigned int position)
{
    memcpy(&memory[position], s, size);
}

void SCC66470::WriteToBIOSArea(const void* s, unsigned int size, unsigned int position)
{
    PutDataInMemory(s, size, position + 0x180000);
}

void SCC66470::SetOnFrameCompletedCallback(std::function<void()> callback)
{
    OnFrameCompleted = callback;
}

void SCC66470::StopOnNextFrame(const bool stop)
{
    stopOnNextFrame = stop;
}

Plane SCC66470::GetScreen() const
{
    return {nullptr, 0, 0};
}

Plane SCC66470::GetPlaneA() const
{
    return {nullptr, 0, 0};
}

Plane SCC66470::GetPlaneB() const
{
    return {nullptr, 0, 0};
}

Plane SCC66470::GetBackground() const
{
    return {nullptr, 0, 0};
}

Plane SCC66470::GetCursor() const
{
    return {nullptr, 0, 0};
}

#undef   SET_DA_BIT
#undef UNSET_DA_BIT
