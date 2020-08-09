#include "MC68HC05X16.hpp"

#include <fstream>

MC68HC05X16::MC68HC05X16()
{
    memory = new uint8_t[0x4000];
    currentOpcode = 0;
    currentPC = 0;
}

MC68HC05X16::~MC68HC05X16()
{
    delete[] memory;
}

void MC68HC05X16::Reset()
{
    A = 0;
    X = 0;
    PC = (GetByte(0x3FFE) << 8) | GetByte(0x3FFF);
    SP.byte = 0xFF;
    CCR.byte = 0xE0;
}

bool MC68HC05X16::LoadBIOS(const std::string& file)
{
    std::ifstream in(file, std::ios::binary);
    if(!in)
        return false;



    in.close();
    return true;
}

uint8_t MC68HC05X16::GetByte(const uint16_t addr)
{
    if(addr < 0x4000)
        return memory[addr];

    return 0;
}

void MC68HC05X16::SetByte(const uint16_t addr, const uint8_t value)
{
    if(addr < 0x4000)
        memory[addr] = value;
}

uint8_t MC68HC05X16::GetNextByte()
{
    const uint8_t byte = GetByte(PC);
    PC++;
    return byte;
}
