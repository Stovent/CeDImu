#include "SCC66470.hpp"

#include <cstdio>
#include <cstring>

SCC66470::SCC66470(CeDImu* appp) : VDSC(appp)
{
    allocatedMemory = 0x200000;
    memory = new uint8_t[allocatedMemory];
    memset(memory, 0, 1024 * 1024);
    memorySwapCount = 0;
    stopOnNextframe = false;
}

SCC66470::~SCC66470()
{
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

}

void SCC66470::SetOnFrameCompletedCallback(std::function<void()> callback)
{
    OnFrameCompleted = callback;
}

void SCC66470::StopOnNextFrame(const bool stop)
{
    stopOnNextframe = stop;
}

std::vector<VDSCRegister> SCC66470::GetInternalRegisters()
{
    return std::vector<VDSCRegister>();
}

std::vector<VDSCRegister> SCC66470::GetControlRegisters()
{
    return std::vector<VDSCRegister>();
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
