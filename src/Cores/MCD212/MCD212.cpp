#include "MCD212.hpp"

#include <cstring>
#include <wx/msgdlg.h>

MCD212::MCD212() // TD = 0
{
    memory = new uint8_t[0x500000];
    memorySwapCount = 0;
    allocatedMemory = 0x500000;
    biosLoaded = false;
}

MCD212::~MCD212()
{
    delete[] memory;
}

bool MCD212::LoadBIOS(std::string filename)
{
    biosLoaded = false;
    FILE* f = fopen(filename.c_str(), "rb");
    if(f == NULL)
        return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f); // should be 512ko
    fseek(f, 0, SEEK_SET);

    fread(&memory[0x400000], 1, size, f);

    fclose(f);
    return biosLoaded = true;
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
    memorySwapCount = 0;
}

void MCD212::DisplayLine()
{
    wxMessageBox("Display");
}

uint32_t MCD212::GetLineDisplayTime()
{
    return 700;
}
