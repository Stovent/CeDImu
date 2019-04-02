#include <cstdio>
#include <cstring>

#include "SCC66470.hpp"

SCC66470::SCC66470()
{
    memory = new uint8_t[0x200000 * 2];
    lineNumber = 0;
}

SCC66470::~SCC66470()
{
    delete[] memory;
}

void SCC66470::LoadBIOS(std::string filename) // only CD-I 205, it should be 523 264 bytes long
bool SCC66470::LoadBIOS(std::string filename) // only CD-I 205, it should be 523 264 bytes long
{
    char* c = new char[0x7FBFF];
    FILE* f = fopen(filename.c_str(), "rb");
    if(f == NULL)
        return false;
    fread(c, 1, 0x7FBFF, f);
    memcpy(memory + 0x180000, c, 0x7FBFF);
    return true;
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
