#include "MCD212.hpp"

// internal registers
uint16_t MCD212::GetCSR1RRegister()
{
    return internalRegisters[CSR1R];
}

uint16_t MCD212::GetCSR1WRegister()
{
    return internalRegisters[CSR1W];
}

uint16_t MCD212::GetDCR1Register()
{
    return internalRegisters[DCR1];
}

uint16_t MCD212::GetVSR1Register()
{
    return internalRegisters[VSR1];
}

uint16_t MCD212::GetDDR1Register()
{
    return internalRegisters[DDR1];
}

uint16_t MCD212::GetDCP1Register()
{
    return internalRegisters[DCP1];
}

uint16_t MCD212::GetCSR2RRegister()
{
    return internalRegisters[CSR2R];
}

uint16_t MCD212::GetCSR2WRegister()
{
    return internalRegisters[CSR2W];
}

uint16_t MCD212::GetDCR2Register()
{
    return internalRegisters[DCR2];
}

uint16_t MCD212::GetVSR2Register()
{
    return internalRegisters[VSR2];
}

uint16_t MCD212::GetDDR2Register()
{
    return internalRegisters[DDR2];
}

uint16_t MCD212::GetDCP2Register()
{
    return internalRegisters[DCP2];
}

bool MCD212::GetDA()
{
    return (internalRegisters[CSR1R] & 0x0080) >> 7;
}

bool MCD212::GetPA()
{
    return (internalRegisters[CSR1R] & 0x0020) >> 5;
}

uint8_t MCD212::GetIT12()
{
    return (internalRegisters[CSR2R] & 0x0006) >> 1;
}

bool MCD212::GetBE()
{
    return internalRegisters[CSR2R] & 0x0001;
}

bool MCD212::GetDI1()
{
    return internalRegisters[CSR1W] >> 15;
}

uint8_t MCD212::GetDD12()
{
    return (internalRegisters[CSR1W] & 0x0300) >> 8;
}

bool MCD212::GetTD()
{
    return (internalRegisters[CSR1W] & 0x0020) >> 5;
}

bool MCD212::GetDD()
{
    return (internalRegisters[CSR1W] & 0x0008) >> 3;
}

bool MCD212::GetST()
{
    return (internalRegisters[CSR1W] & 0x0002) >> 1;
}

bool MCD212::GetDI2()
{
    return internalRegisters[CSR2W] >> 15;
}

bool MCD212::GetDE()
{
    return internalRegisters[DCR1] >> 15;
}

bool MCD212::GetCF()
{
    return (internalRegisters[DCR1] & 0x4000) >> 14;
}

bool MCD212::GetFD()
{
    return (internalRegisters[DCR1] & 0x2000) >> 13;
}

bool MCD212::GetSM()
{
    return (internalRegisters[DCR1] & 0x1000) >> 12;
}

bool MCD212::GetCM1()
{
    return (internalRegisters[DCR1] & 0x0800) >> 11;
}

bool MCD212::GetIC1()
{
    return (internalRegisters[DCR1] & 0x0200) >> 9;
}

bool MCD212::GetDC1()
{
    return (internalRegisters[DCR1] & 0x0100) >> 8;
}

bool MCD212::GetCM2()
{
    return (internalRegisters[DCR2] & 0x0800) >> 11;
}

bool MCD212::GetIC2()
{
    return (internalRegisters[DCR2] & 0x0200) >> 9;
}

bool MCD212::GetDC2()
{
    return (internalRegisters[DCR2] & 0x0100) >> 8;
}

uint8_t MCD212::GetMF12_1() // 2*2^MF
{
    return (internalRegisters[DDR1] & 0x0C00) >> 10;
}

uint8_t MCD212::GetFT12_1()
{
    return (internalRegisters[DDR1] & 0x0300) >> 8;
}

uint8_t MCD212::GetMF12_2()
{
    return (internalRegisters[DDR2] & 0x0C00) >> 10;
}

uint8_t MCD212::GetFT12_2()
{
    return (internalRegisters[DDR2] & 0x0300) >> 8;
}

uint32_t MCD212::GetVSR1()
{
    return (internalRegisters[DCR1] & 0x003F) << 16 | internalRegisters[VSR1];
}

uint32_t MCD212::GetVSR2()
{
    return (internalRegisters[DCR2] & 0x003F) << 16 | internalRegisters[VSR2];
}

uint32_t MCD212::GetDCP1()
{
    return (internalRegisters[DDR1] & 0x003F) << 16 | internalRegisters[DCP1];
}

uint32_t MCD212::GetDCP2()
{
    return (internalRegisters[DDR2] & 0x003F) << 16 | internalRegisters[DCP2];
}

void MCD212::SetIT1(const bool it)
{
    internalRegisters[CSR1R] &= 0x0003;
    internalRegisters[CSR1R] |= it << 2;
}

void MCD212::SetIT2(const bool it)
{
    internalRegisters[CSR1R] &= 0x0005;
    internalRegisters[CSR1R] |= it << 1;
}

void MCD212::SetDCP1(const uint32_t value)
{
    internalRegisters[DDR1] &= 0x0F00;
    internalRegisters[DDR1] |= (value >> 16) & 0x3F;
    internalRegisters[DCP1] = value;
}

void MCD212::SetVSR1(const uint32_t value)
{
    internalRegisters[DCR1] &= 0xFB00;
    internalRegisters[DCR1] |= (value >> 16) & 0x3F;
    internalRegisters[VSR1] = value;
}

void MCD212::SetDCP2(const uint32_t value)
{
    internalRegisters[DDR2] &= 0x0F00;
    internalRegisters[DDR2] |= (value >> 16) & 0x3F;
    internalRegisters[DCP2] = value;
}

void MCD212::SetVSR2(const uint32_t value)
{
    internalRegisters[DCR2] &= 0x0B00;
    internalRegisters[DCR2] |= (value >> 16) & 0x3F;
    internalRegisters[VSR2] = value;
}
