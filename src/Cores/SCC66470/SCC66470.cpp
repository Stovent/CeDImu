#include <fstream>

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
{
    char* c = new char[0x7FBFF];
    std::ifstream in(filename, std::ios::binary);
    in.get(c, 0x7FBFF);
    memcpy(&(memory[0x180000]), c, 0x7FBFF);
}

void SCC66470::PutDataInMemory(const uint8_t* s, unsigned int size, unsigned int position)
{
    memcpy(&memory[position], s, size);
}

void SCC66470::DisplayLine()
{

}
