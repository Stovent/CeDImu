#include "SCC68070.hpp"

#include "../../utils.h"

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
