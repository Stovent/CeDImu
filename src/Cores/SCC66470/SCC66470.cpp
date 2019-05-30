#include <cstdio>
#include <cstring>

#include "SCC66470.hpp"

SCC66470::SCC66470()
{
    biosLoaded = false;
    memory = new uint8_t[0x2FFFFF];
    lineNumber = 0;
    allocatedMemory = 0x300000;
    memorySwapCount = 0;
}

SCC66470::~SCC66470()
{
    delete[] memory;
}

void SCC66470::MemorySwap()
{
    memorySwapCount = 0;
}

bool SCC66470::LoadBIOS(std::string filename) // only CD-I 205, it should be 523 264 bytes long
{
    biosLoaded = false;
    FILE* f = fopen(filename.c_str(), "rb");
    if(f == NULL)
        return false;
    fread(memory + 0x180000, 1, 523264, f);
    fclose(f);
    return biosLoaded = true;
}

void SCC66470::PutDataInMemory(const uint8_t* s, unsigned int size, unsigned int position)
{
    memcpy(&memory[position], s, size);
}

void SCC66470::ResetMemory()
{
    memset(memory, 0, 1024 * 1024);
}

void SCC66470::DisplayLine()
{

}

uint32_t SCC66470::GetLineDisplayTime()
{
    return 700;
}
