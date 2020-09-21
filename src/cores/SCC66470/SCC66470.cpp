#include "SCC66470.hpp"
#include "../../Boards/Board.hpp"
#include "../../utils.hpp"

#include <cstdio>
#include <cstring>

#define   SET_DA_BIT() internalRegisters[SCSRR] |= 0x80;
#define UNSET_DA_BIT() internalRegisters[SCSRR] &= 0x67;

SCC66470::SCC66470(Board* board, const bool ismaster) : VDSC(board), isMaster(ismaster)
{
    memorySwapCount = 0;
    stopOnNextFrame = false;
    allocatedMemory = 0x200000;

    OPEN_LOG(out_dram, isMaster ? "SCC66470_master_DRAM.txt" : "SCC66470_slave_DRAM.txt")

    memory = new uint8_t[allocatedMemory];
    memset(memory, 0, 1024 * 1024);
    internalRegisters = new uint16_t[0x20];
    memset(internalRegisters, 0, 0x20);
    Reset();
}

SCC66470::~SCC66470()
{
    delete[] memory;
    delete[] internalRegisters;
}

void SCC66470::Reset()
{
    MemorySwap();
}

void SCC66470::MemorySwap()
{
    memorySwapCount = 0;
}

bool SCC66470::LoadBIOS(const void* bios, const uint32_t size) // only CD-I 205, it should be 523 264 bytes long
{
    if(size > 0x7FC00)
    {
        return biosLoaded = false;
    }

    memcpy(&memory[0x180000], bios, size);

    return biosLoaded = true;
}

void SCC66470::PutDataInMemory(const void* s, unsigned int size, unsigned int position)
{
    memcpy(&memory[position], s, size);
}

void SCC66470::DrawLine()
{
    SET_DA_BIT()

//    if(++lineNumber >= GetVerticalResolution())
    {
        UNSET_DA_BIT()

        if(OnFrameCompleted)
            OnFrameCompleted();

        lineNumber = 0;
        totalFrameCount++;
        if(stopOnNextFrame)
        {
            board->cpu->Stop(false);
            stopOnNextFrame = false;
        }
    }
}

void SCC66470::SetOnFrameCompletedCallback(std::function<void()> callback)
{
    OnFrameCompleted = callback;
}

void SCC66470::StopOnNextFrame(const bool stop)
{
    stopOnNextFrame = stop;
}

wxImage SCC66470::GetScreen()
{
    return wxImage();
}

wxImage SCC66470::GetPlaneA()
{
    return wxImage();
}

wxImage SCC66470::GetPlaneB()
{
    return wxImage();
}

wxImage SCC66470::GetBackground()
{
    return wxImage();
}

wxImage SCC66470::GetCursor()
{
    return wxImage();
}

#undef   SET_DA_BIT
#undef UNSET_DA_BIT
