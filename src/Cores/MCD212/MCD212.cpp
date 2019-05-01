#include "MCD212.hpp"

#include <cstring>

MCD212::MCD212() // TD = 0
{
    memory = new uint16_t[0x4FFFFF];
    memorySwap = 0;
}

MCD212::~MCD212()
{
    delete[] memory;
}

bool MCD212::LoadBIOS(std::string filename)
{
    FILE* f = fopen(filename.c_str(), "rb");
    if(f == NULL)
        return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    fread(&memory[0x400000], 2, size/2, f);

    fclose(f);

    f = fopen("D:\\mem.bin", "wb");
    fwrite(memory, 2, 0x500000, f);
    fclose(f);
    return true;
}

void MCD212::PutDataInMemory(const uint8_t* s, unsigned int size, unsigned int position)
{
    memcpy(&memory[position], s, size);
}

void MCD212::ResetMemory()
{
    memset(memory, 0, 0x7FFFF);
}

void MCD212::MemorySwap()
{
    memorySwap = 0;
}

void MCD212::DisplayLine()
{

}
