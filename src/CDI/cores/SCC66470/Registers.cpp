#include "SCC66470.hpp"

uint16_t SCC66470::GetCSRWRegister() const
{
    return internalRegisters[SCSRW];
}

uint16_t SCC66470::GetCSRRRegister() const
{
    return registerCSR;
}

uint16_t SCC66470::GetDCRRegister() const
{
    return internalRegisters[SDCR];
}

uint16_t SCC66470::GetVSRRegister() const
{
    return internalRegisters[SVSR];
}

uint16_t SCC66470::GetBCRRegister() const
{
    return internalRegisters[SBCR];
}

uint16_t SCC66470::GetDCR2Register() const
{
    return internalRegisters[SDCR2];
}

uint16_t SCC66470::GetDCPRegister() const
{
    return internalRegisters[SDCP];
}

uint16_t SCC66470::GetSWMRegister() const
{
    return internalRegisters[SSWM];
}

uint16_t SCC66470::GetSTMRegister() const
{
    return internalRegisters[SSTM];
}

uint16_t SCC66470::GetARegister() const
{
    return internalRegisters[SA];
}

uint16_t SCC66470::GetBRegister() const
{
    return internalRegisters[SB];
}

uint16_t SCC66470::GetPCRRegister() const
{
    return internalRegisters[SPCR];
}

uint16_t SCC66470::GetMASKRegister() const
{
    return internalRegisters[SMASK];
}

uint16_t SCC66470::GetSHIFTRegister() const
{
    return internalRegisters[SSHIFT];
}

uint16_t SCC66470::GetINDEXRegister() const
{
    return internalRegisters[SINDEX];
}

uint16_t SCC66470::GetFCRegister() const
{
    return internalRegisters[SFCBC] & 0xFF00;
}

uint16_t SCC66470::GetBCRegister() const
{
    return internalRegisters[SFCBC] & 0x00FF;
}

uint16_t SCC66470::GetTCRegister() const
{
    return internalRegisters[STC];
}

bool SCC66470::GetFD() const
{
    return internalRegisters[SDCR] & 0x1000;
}

bool SCC66470::GetSS() const
{
    return internalRegisters[SDCR] & 0x0400;
}

bool SCC66470::GetST() const
{
    return internalRegisters[SCSRW] & 0x0002;
}

std::vector<VDSCRegister> SCC66470::GetInternalRegisters() const
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

std::vector<VDSCRegister> SCC66470::GetControlRegisters() const
{
    return std::vector<VDSCRegister>();
}
