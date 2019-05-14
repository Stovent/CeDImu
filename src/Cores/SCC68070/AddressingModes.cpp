#include "SCC68070.hpp"

#include "../../utils.hpp"

int32_t SCC68070::GetIndexRegister(const uint16_t& bew)
{
    if(bew & 0x8000)
        if(bew & 0x0800)
            return A[bew & 0x7000];
        else
            return signExtend16(A[bew & 0x7000]);
    else
        if(bew & 0x0800)
            return D[bew & 0x7000];
        else
            return signExtend16(D[bew & 0x7000]);
}

uint32_t SCC68070::AddressRegisterIndirectWithPostincrement(const uint8_t& reg, const uint8_t& sizeInByte)
{
    int32_t addr = A[reg];
    A[reg] += (reg == 7 && sizeInByte == 1) ? 2 : sizeInByte;
    return addr;
}

uint32_t SCC68070::AddressRegisterIndirectWithPredecrement(const uint8_t& reg, const uint8_t& sizeInByte)
{
    A[reg] -= (reg == 7 && sizeInByte == 1) ? 2 : sizeInByte;
    return A[reg];
}

uint32_t SCC68070::AddressRegisterIndirectWithDisplacement(const uint8_t& reg)
{
    return A[reg] + signExtend16(GetNextWord());
}

uint32_t SCC68070::AddressRegisterIndirectWithIndex8(const uint8_t& reg)
{
    uint16_t bew = GetNextWord();
    return A[reg] + GetIndexRegister(bew) + signExtend8(bew & 0x00FF);
}

uint32_t SCC68070::ProgramCounterIndirectWithDisplacement()
{
    return PC + signExtend16(GetNextWord());
}

uint32_t SCC68070::ProgramCounterIndirectWithIndex8()
{
    uint16_t bew = GetNextWord();
    return PC + GetIndexRegister(bew) + signExtend8(bew & 0x00FF);
}

uint32_t SCC68070::AbsoluteShortAddressing()
{
    return signExtend16(GetNextWord());
}

uint32_t SCC68070::AbsoluteLongAddressing()
{
    return GetNextWord() << 16 | GetNextWord();
}

std::string SCC68070::DisassembleAddressingMode(uint32_t extWordAddress, uint8_t eamode, uint8_t eareg, uint8_t size)
{
    std::string mode;
    if(eamode == 0)
    {
        mode = "D" + std::to_string(eareg);
    }
    else if(eamode == 1)
    {
        mode = "A" + std::to_string(eareg);
    }
    else if(eamode == 2)
    {
        mode = "(A" + std::to_string(eareg) + ")";
    }
    else if(eamode == 3)
    {
        mode = "(A" + std::to_string(eareg) + ")+";
    }
    else if(eamode == 4)
    {
        mode = "-(A" + std::to_string(eareg) + ")";
    }
    else if(eamode == 5)
    {
        mode = "(" + std::to_string((int16_t)vdsc.GetWord(extWordAddress)) + ",A" + std::to_string(eareg) + ")";
    }
    else if(eamode == 6)
    {
        uint16_t bew = vdsc.GetWord(extWordAddress);
        mode = "(" + std::to_string((int8_t)bew) + ",A" + std::to_string(eareg) + ((bew & 0x8000) ? ",A" : ",D") + std::to_string((bew & 0x7000) >> 12) + ")";
    }
    else if(eamode == 7)
    {
        if(eareg == 0)
        {
            mode = "(" + toHex(vdsc.GetWord(extWordAddress)) + ").W";
        }
        else if(eareg == 1)
        {
            mode = "(" + toHex(vdsc.GetLong(extWordAddress)) + ").L";
        }
        else if(eareg == 2)
        {
            mode = "(" + std::to_string((int16_t)vdsc.GetWord(extWordAddress)) + ",PC)";
        }
        else if(eareg == 3)
        {
            uint16_t bew = vdsc.GetWord(extWordAddress);
            mode = "(" + std::to_string((int8_t)bew) + ",PC," + ((bew & 0x8000) ? "A" : "D") + std::to_string((bew & 0x7000) >> 12) + ")";
        }
        else if(eareg == 4)
        {
            mode = "#" + ((size == 1) ? std::to_string(vdsc.GetWord(extWordAddress) & 0x00FF) : (size == 2) ? std::to_string(vdsc.GetWord(extWordAddress)) : (size == 4) ? std::to_string(vdsc.GetLong(extWordAddress)) : "ERROR");
        }
        else
            mode = "Wrong register for mode 7";
    }
    else
    {
        mode = "Unknown addressing mode";
    }
    return mode;
}
