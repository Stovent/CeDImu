#include <cstdio>
#include <cstring>

#include "SCC66470.hpp"

SCC66470::SCC66470(CeDImu* appp) : VDSC(appp)
{
    allocatedMemory = 0x200000;
    memory = new uint8_t[allocatedMemory];
    memset(memory, 0, 1024 * 1024);
    memorySwapCount = 0;
}

SCC66470::~SCC66470()
{

}

void SCC66470::Reset()
{

}

void SCC66470::MemorySwap()
{
    memorySwapCount = 0;
}

bool SCC66470::LoadBIOS(const char* filename) // only CD-I 205, it should be 523 264 bytes long
{
    biosLoaded = false;
    FILE* f = fopen(filename, "rb");
    if(f == NULL)
        return false;
    fread(memory + 0x180000, 1, 523264, f);
    fclose(f);
    return biosLoaded = true;
}

void SCC66470::PutDataInMemory(const void* s, unsigned int size, unsigned int position)
{
    memcpy(&memory[position], s, size);
}

void SCC66470::DisplayLine()
{

}

void SCC66470::OnFrameCompleted()
{

}
