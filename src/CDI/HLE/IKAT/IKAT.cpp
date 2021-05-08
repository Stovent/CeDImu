#include "IKAT.hpp"

namespace HLE
{

#define UNSET_RDIDLE(var) var &= 0x01;
#define UNSET_WRIDLE(var) var &= 0x10;
#define SET_RDWRIDLE(var) var = 0x11;

IKAT::IKAT(SCC68070& cpu, const bool PAL) : ISlave(cpu)
{
    responseCF6[2] = PAL + 1;

    for(int i = PASR; i <= PDSR; i++)
        SET_RDWRIDLE(registers[i])
}

uint8_t IKAT::GetByte(const uint8_t addr)
{
    if(addr >= ISR)
    {
        for(int i = PA; i <= PD; i++)
            if(responsesIterator[i] != responsesEnd[i])
                UNSET_RDIDLE(registers[PASR + i])
            else
                SET_RDWRIDLE(registers[PASR + i])

        return registers[addr];
    }

    const uint8_t reg = addr % 4;
    if(addr >= PARD && addr <= PDRD && responsesIterator[reg] != responsesEnd[reg])
    {
        registers[PARD + reg] = *responsesIterator[reg]++;
    }

    return registers[addr];
}

void IKAT::SetByte(const uint8_t addr, const uint8_t data)
{
    registers[addr] = data;

    switch(addr)
    {
    case PCWR:
        ProcessCommandC(data);
        break;

    case PDWR:
        ProcessCommandD(data);
        break;
    }
}

void IKAT::ProcessCommandC(uint8_t data)
{
    switch(data)
    {
    case 0xF4:
        responsesIterator[PC] = responseCF4.begin();
        responsesEnd[PC] = responseCF4.end();
        UNSET_WRIDLE(registers[PCSR])
        break;

    case 0xF6:
        responsesIterator[PC] = responseCF6.begin();
        responsesEnd[PC] = responseCF6.end();
        UNSET_WRIDLE(registers[PCSR])
        break;
    }
}

void IKAT::ProcessCommandD(uint8_t data)
{
    switch(data)
    {
    case 0xB0:
        responsesIterator[PD] = responseDB0.begin();
        responsesEnd[PD] = responseDB0.end();
        UNSET_WRIDLE(registers[PDSR])
        break;

    case 0xB1:
        responsesIterator[PD] = responseDB1.begin();
        responsesEnd[PD] = responseDB1.end();
        UNSET_WRIDLE(registers[PDSR])
        break;

    case 0xB2:
        responsesIterator[PD] = responseDB2.begin();
        responsesEnd[PD] = responseDB2.end();
        UNSET_WRIDLE(registers[PDSR])
        break;
    }
}

} // namespace HLE
