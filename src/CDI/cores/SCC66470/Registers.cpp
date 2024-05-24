#include "SCC66470.hpp"

uint16_t SCC66470::GetCSRWRegister() const
{
    return internalRegisters[CSRW];
}

uint16_t SCC66470::GetCSRRRegister() const
{
    return registerCSR;
}

uint16_t SCC66470::GetDCRRegister() const
{
    return internalRegisters[DCR];
}

uint16_t SCC66470::GetVSRRegister() const
{
    return internalRegisters[VSR];
}

uint16_t SCC66470::GetBCRRegister() const
{
    return internalRegisters[BCR];
}

uint16_t SCC66470::GetDCR2Register() const
{
    return internalRegisters[DCR2];
}

uint16_t SCC66470::GetDCPRegister() const
{
    return internalRegisters[DCP];
}

uint16_t SCC66470::GetSWMRegister() const
{
    return internalRegisters[SWM];
}

uint16_t SCC66470::GetSTMRegister() const
{
    return internalRegisters[STM];
}

uint16_t SCC66470::GetARegister() const
{
    return internalRegisters[A];
}

uint16_t SCC66470::GetBRegister() const
{
    return internalRegisters[B];
}

uint16_t SCC66470::GetPCRRegister() const
{
    return internalRegisters[PCR];
}

uint16_t SCC66470::GetMASKRegister() const
{
    return internalRegisters[MASK];
}

uint16_t SCC66470::GetSHIFTRegister() const
{
    return internalRegisters[SHIFT];
}

uint16_t SCC66470::GetINDEXRegister() const
{
    return internalRegisters[INDEX];
}

uint16_t SCC66470::GetFCRegister() const
{
    return internalRegisters[FCBC] & 0xFF00;
}

uint16_t SCC66470::GetBCRegister() const
{
    return internalRegisters[FCBC] & 0x00FF;
}

uint16_t SCC66470::GetTCRegister() const
{
    return internalRegisters[TC];
}

bool SCC66470::GetFD() const
{
    return internalRegisters[DCR] & 0x1000;
}

bool SCC66470::GetSS() const
{
    return internalRegisters[DCR] & 0x0400;
}

bool SCC66470::GetST() const
{
    return internalRegisters[CSRW] & 0x0002;
}

std::vector<InternalRegister> SCC66470::GetInternalRegisters() const
{
    const uint32_t base = isMaster ? 0x1FFFE0 : 0x1FFFC0;
    return {
        {"SCSRW",  0x00 + base, GetCSRWRegister(), ""},
        {"SCSRR",  0x01 + base, GetCSRRRegister(), ""},
        {"SDCR",   0x02 + base, GetDCRRegister(), ""},
        {"SVSR",   0x04 + base, GetVSRRegister(), ""},
        {"SBCR",   0x07 + base, GetBCRRegister(), ""},
        {"SDCR2",  0x08 + base, GetDCR2Register(), ""},
        {"SDCP",   0x0A + base, GetDCPRegister(), ""},
        {"SSWM",   0x0C + base, GetSWMRegister(), ""},
        {"SSTM",   0x0F + base, GetSTMRegister(), ""},
        {"SA",     0x10 + base, GetARegister(), ""},
        {"SB",     0x12 + base, GetBRegister(), ""},
        {"SPCR",   0x14 + base, GetPCRRegister(), ""},
        {"SMASK",  0x17 + base, GetMASKRegister(), ""},
        {"SSHIFT", 0x18 + base, GetSHIFTRegister(), ""},
        {"SINDEX", 0x1B + base, GetINDEXRegister(), ""},
        {"SFC",    0x1C + base, GetFCRegister(), ""},
        {"SBC",    0x1D + base, GetBCRegister(), ""},
        {"STC",    0x1E + base, GetTCRegister(), ""},
    };
}

std::vector<InternalRegister> SCC66470::GetControlRegisters() const
{
    return std::vector<::InternalRegister>();
}
