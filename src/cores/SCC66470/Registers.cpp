#include "SCC66470.hpp"

uint16_t SCC66470::GetCSRWRegister()
{
    return internalRegisters[SCSRW];
}

uint16_t SCC66470::GetCSRRRegister()
{
    return internalRegisters[SCSRR];
}

uint16_t SCC66470::GetDCRRegister()
{
    return internalRegisters[SDCR];
}

uint16_t SCC66470::GetVSRRegister()
{
    return internalRegisters[SVSR];
}

uint16_t SCC66470::GetBCRRegister()
{
    return internalRegisters[SBCR];
}

uint16_t SCC66470::GetDCR2Register()
{
    return internalRegisters[SDCR2];
}

uint16_t SCC66470::GetDCPRegister()
{
    return internalRegisters[SDCP];
}

uint16_t SCC66470::GetSWMRegister()
{
    return internalRegisters[SSWM];
}

uint16_t SCC66470::GetSTMRegister()
{
    return internalRegisters[SSTM];
}

uint16_t SCC66470::GetARegister()
{
    return internalRegisters[SA];
}

uint16_t SCC66470::GetBRegister()
{
    return internalRegisters[SB];
}

uint16_t SCC66470::GetPCRRegister()
{
    return internalRegisters[SPCR];
}

uint16_t SCC66470::GetMASKRegister()
{
    return internalRegisters[SMASK];
}

uint16_t SCC66470::GetSHIFTRegister()
{
    return internalRegisters[SSHIFT];
}

uint16_t SCC66470::GetINDEXRegister()
{
    return internalRegisters[SINDEX];
}

uint16_t SCC66470::GetFCRegister()
{
    return internalRegisters[SFC];
}

uint16_t SCC66470::GetBCRegister()
{
    return internalRegisters[SBC];
}

uint16_t SCC66470::GetTCRegister()
{
    return internalRegisters[STC];
}

std::vector<VDSCRegister> SCC66470::GetInternalRegisters()
{
    std::vector<VDSCRegister> registers;
    uint32_t base = isMaster ? 0x1FFFE0 : 0x1FFFC0;
    registers.push_back(VDSCRegister({"SCSRW",  SCSRW + base,  GetCSRWRegister(), ""}));
    registers.push_back(VDSCRegister({"SCSRR",  SCSRR + base,  GetCSRRRegister(), ""}));
    registers.push_back(VDSCRegister({"SDCR",   SDCR + base,   GetDCRRegister(), ""}));
    registers.push_back(VDSCRegister({"SVSR",   SVSR + base,   GetVSRRegister(), ""}));
    registers.push_back(VDSCRegister({"SBCR",   SBCR + base,   GetBCRRegister(), ""}));
    registers.push_back(VDSCRegister({"SDCR2",  SDCR2 + base,  GetDCR2Register(), ""}));
    registers.push_back(VDSCRegister({"SDCP",   SDCP + base,   GetDCPRegister(), ""}));
    registers.push_back(VDSCRegister({"SSWM",   SSWM + base,   GetSWMRegister(), ""}));
    registers.push_back(VDSCRegister({"SSTM",   SSTM + base,   GetSTMRegister(), ""}));
    registers.push_back(VDSCRegister({"SA",     SA + base,     GetARegister(), ""}));
    registers.push_back(VDSCRegister({"SB",     SB + base,     GetBRegister(), ""}));
    registers.push_back(VDSCRegister({"SPCR",   SPCR + base,   GetPCRRegister(), ""}));
    registers.push_back(VDSCRegister({"SMASK",  SMASK + base,  GetMASKRegister(), ""}));
    registers.push_back(VDSCRegister({"SSHIFT", SSHIFT + base, GetSHIFTRegister(), ""}));
    registers.push_back(VDSCRegister({"SINDEX", SINDEX + base, GetINDEXRegister(), ""}));
    registers.push_back(VDSCRegister({"SFC",    SFC + base,    GetFCRegister(), ""}));
    registers.push_back(VDSCRegister({"SBC",    SBC + base,    GetBCRegister(), ""}));
    registers.push_back(VDSCRegister({"STC",    STC + base,    GetTCRegister(), ""}));
    return registers;
}

std::vector<VDSCRegister> SCC66470::GetControlRegisters()
{
    return std::vector<VDSCRegister>();
}
