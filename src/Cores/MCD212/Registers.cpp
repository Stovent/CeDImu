#include "MCD212.hpp"

uint16_t MCD212::GetCSR1RRegister()
{
    return registers[CSR1R];
}

uint16_t MCD212::GetCSR1WRegister()
{
    return registers[CSR1W];
}

uint16_t MCD212::GetDCR1Register()
{
    return registers[DCR1];
}

uint16_t MCD212::GetVSR1Register()
{
    return registers[VSR1];
}

uint16_t MCD212::GetDDR1Register()
{
    return registers[DDR1];
}

uint16_t MCD212::GetDCP1Register()
{
    return registers[DCP1];
}

uint16_t MCD212::GetCSR2RRegister()
{
    return registers[CSR2R];
}

uint16_t MCD212::GetCSR2WRegister()
{
    return registers[CSR2W];
}

uint16_t MCD212::GetDCR2Register()
{
    return registers[DCR2];
}

uint16_t MCD212::GetVSR2Register()
{
    return registers[VSR2];
}

uint16_t MCD212::GetDDR2Register()
{
    return registers[DDR2];
}

uint16_t MCD212::GetDCP2Register()
{
    return registers[DCP2];
}

bool MCD212::GetDA()
{
    return (registers[CSR1R] & 0x0080) >> 7;
}

bool MCD212::GetPA()
{
    return (registers[CSR1R] & 0x0020) >> 5;
}

uint8_t MCD212::GetIT12()
{
    return (registers[CSR2R] & 0x0006) >> 1;
}

bool MCD212::GetBE()
{
    return registers[CSR2R] & 0x0001;
}

bool MCD212::GetDI1()
{
    return registers[CSR1W] >> 15;
}

uint8_t MCD212::GetDD12()
{
    return (registers[CSR1W] & 0x0300) >> 8;
}

bool MCD212::GetTD()
{
    return (registers[CSR1W] & 0x0020) >> 5;
}

bool MCD212::GetDD()
{
    return (registers[CSR1W] & 0x0008) >> 3;
}

bool MCD212::GetST()
{
    return (registers[CSR1W] & 0x0002) >> 1;
}

bool MCD212::GetDI2()
{
    return registers[CSR2W] >> 15;
}

bool MCD212::GetDE()
{
    return registers[DCR1] >> 15;
}

bool MCD212::GetCF()
{
    return (registers[DCR1] & 0x4000) >> 14;
}

bool MCD212::GetFD()
{
    return (registers[DCR1] & 0x2000) >> 13;
}

bool MCD212::GetSM()
{
    return (registers[DCR1] & 0x1000) >> 12;
}

bool MCD212::GetCM1()
{
    return (registers[DCR1] & 0x0800) >> 11;
}

bool MCD212::GetIC1()
{
    return (registers[DCR1] & 0x0200) >> 9;
}

bool MCD212::GetDC1()
{
    return (registers[DCR1] & 0x0100) >> 8;
}

bool MCD212::GetCM2()
{
    return (registers[DCR2] & 0x0800) >> 11;
}

bool MCD212::GetIC2()
{
    return (registers[DCR2] & 0x0200) >> 9;
}

bool MCD212::GetDC2()
{
    return (registers[DCR2] & 0x0100) >> 8;
}

uint8_t MCD212::GetMF12_1()
{
    return (registers[DDR1] & 0x0C00) >> 10;
}

uint8_t MCD212::GetFT12_1()
{
    return (registers[DDR1] & 0x0300) >> 8;
}

uint8_t MCD212::GetMF12_2()
{
    return (registers[DDR2] & 0x0C00) >> 10;
}

uint8_t MCD212::GetFT12_2()
{
    return (registers[DDR2] & 0x0300) >> 8;
}

uint32_t MCD212::GetVSR1()
{
    return (registers[DCR1] & 0x003F) << 16 | registers[VSR1];
}

uint32_t MCD212::GetVSR2()
{
    return (registers[DCR2] & 0x003F) << 16 | registers[VSR2];
}

uint32_t MCD212::GetDCP1()
{
    return (registers[DDR1] & 0x003F) << 16 | registers[DCP1];
}

uint32_t MCD212::GetDCP2()
{
    return (registers[DDR2] & 0x003F) << 16 | registers[DCP2];
}
