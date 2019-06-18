#include "MCD212.hpp"

uint16_t MCD212::GetCSR1R()
{
    return registers[CSR1R];
}

uint16_t MCD212::GetCSR1W()
{
    return registers[CSR1W];
}

uint16_t MCD212::GetDCR1()
{
    return registers[DCR1];
}

uint16_t MCD212::GetVSR1()
{
    return registers[VSR1];
}

uint16_t MCD212::GetDDR1()
{
    return registers[DDR1];
}

uint16_t MCD212::GetDCP1()
{
    return registers[DCP1];
}

uint16_t MCD212::GetCSR2R()
{
    return registers[CSR2R];
}

uint16_t MCD212::GetCSR2W()
{
    return registers[CSR2W];
}

uint16_t MCD212::GetDCR2()
{
    return registers[DCR2];
}

uint16_t MCD212::GetVSR2()
{
    return registers[VSR2];
}

uint16_t MCD212::GetDDR2()
{
    return registers[DDR2];
}

uint16_t MCD212::GetDCP2()
{
    return registers[DCP2];
}
