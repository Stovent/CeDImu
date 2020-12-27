#include "MCD212.hpp"
#include "../../utils.hpp"

#include <cmath>

// internal registers
uint16_t MCD212::GetCSR1RRegister() const
{
    return internalRegisters[MCSR1R];
}

uint16_t MCD212::GetCSR1WRegister() const
{
    return internalRegisters[MCSR1W];
}

uint16_t MCD212::GetDCR1Register() const
{
    return internalRegisters[MDCR1];
}

uint16_t MCD212::GetVSR1Register() const
{
    return internalRegisters[MVSR1];
}

uint16_t MCD212::GetDDR1Register() const
{
    return internalRegisters[MDDR1];
}

uint16_t MCD212::GetDCP1Register() const
{
    return internalRegisters[MDCP1];
}

uint16_t MCD212::GetCSR2RRegister() const
{
    return internalRegisters[MCSR2R];
}

uint16_t MCD212::GetCSR2WRegister() const
{
    return internalRegisters[MCSR2W];
}

uint16_t MCD212::GetDCR2Register() const
{
    return internalRegisters[MDCR2];
}

uint16_t MCD212::GetVSR2Register() const
{
    return internalRegisters[MVSR2];
}

uint16_t MCD212::GetDDR2Register() const
{
    return internalRegisters[MDDR2];
}

uint16_t MCD212::GetDCP2Register() const
{
    return internalRegisters[MDCP2];
}

bool MCD212::GetDA() const
{
    return (internalRegisters[MCSR1R] & 0x0080) >> 7;
}

bool MCD212::GetPA() const
{
    return (internalRegisters[MCSR1R] & 0x0020) >> 5;
}

bool MCD212::GetIT1() const
{
    return (internalRegisters[MCSR2R] & 0x0004) >> 2;
}

bool MCD212::GetIT2() const
{
    return (internalRegisters[MCSR2R] & 0x0002) >> 1;
}

bool MCD212::GetBE_R() const
{
    return internalRegisters[MCSR2R] & 0x0001;
}

bool MCD212::GetBE_W() const
{
    return internalRegisters[MCSR1W] & 0x0001;
}

bool MCD212::GetDI1() const
{
    return internalRegisters[MCSR1W] >> 15;
}

uint8_t MCD212::GetDD12() const
{
    return (internalRegisters[MCSR1W] & 0x0300) >> 8;
}

bool MCD212::GetTD() const
{
    return (internalRegisters[MCSR1W] & 0x0020) >> 5;
}

bool MCD212::GetDD() const
{
    return (internalRegisters[MCSR1W] & 0x0008) >> 3;
}

bool MCD212::GetST() const
{
    return (internalRegisters[MCSR1W] & 0x0002) >> 1;
}

bool MCD212::GetDI2() const
{
    return internalRegisters[MCSR2W] >> 15;
}

bool MCD212::GetDE() const
{
    return internalRegisters[MDCR1] >> 15;
}

bool MCD212::GetCF() const
{
    return (internalRegisters[MDCR1] & 0x4000) >> 14;
}

bool MCD212::GetFD() const
{
    return (internalRegisters[MDCR1] & 0x2000) >> 13;
}

bool MCD212::GetSM() const
{
    return (internalRegisters[MDCR1] & 0x1000) >> 12;
}

bool MCD212::GetCM1() const
{
    return (internalRegisters[MDCR1] & 0x0800) >> 11;
}

bool MCD212::GetIC1() const
{
    return (internalRegisters[MDCR1] & 0x0200) >> 9;
}

bool MCD212::GetDC1() const
{
    return (internalRegisters[MDCR1] & 0x0100) >> 8;
}

bool MCD212::GetCM2() const
{
    return (internalRegisters[MDCR2] & 0x0800) >> 11;
}

bool MCD212::GetIC2() const
{
    return (internalRegisters[MDCR2] & 0x0200) >> 9;
}

bool MCD212::GetDC2() const
{
    return (internalRegisters[MDCR2] & 0x0100) >> 8;
}

uint8_t MCD212::GetMF12_1() const // 2*2^MF
{
    return (internalRegisters[MDDR1] & 0x0C00) >> 10;
}

uint8_t MCD212::GetFT12_1() const
{
    return (internalRegisters[MDDR1] & 0x0300) >> 8;
}

uint8_t MCD212::GetMF12_2() const
{
    return (internalRegisters[MDDR2] & 0x0C00) >> 10;
}

uint8_t MCD212::GetFT12_2() const
{
    return (internalRegisters[MDDR2] & 0x0300) >> 8;
}

uint32_t MCD212::GetVSR1() const
{
    return (internalRegisters[MDCR1] & 0x003F) << 16 | internalRegisters[MVSR1];
}

uint32_t MCD212::GetVSR2() const
{
    return (internalRegisters[MDCR2] & 0x003F) << 16 | internalRegisters[MVSR2];
}

uint32_t MCD212::GetDCP1() const
{
    return (internalRegisters[MDDR1] & 0x003F) << 16 | internalRegisters[MDCP1];
}

uint32_t MCD212::GetDCP2() const
{
    return (internalRegisters[MDDR2] & 0x003F) << 16 | internalRegisters[MDCP2];
}

void MCD212::SetIT1(const bool it)
{
    internalRegisters[MCSR1R] &= 0x0003;
    internalRegisters[MCSR1R] |= it << 2;
}

void MCD212::SetIT2(const bool it)
{
    internalRegisters[MCSR1R] &= 0x0005;
    internalRegisters[MCSR1R] |= it << 1;
}

void MCD212::SetDCP1(const uint32_t value)
{
    internalRegisters[MDDR1] &= 0x0F00;
    internalRegisters[MDDR1] |= (value >> 16) & 0x3F;
    internalRegisters[MDCP1] = value;
}

void MCD212::SetVSR1(const uint32_t value)
{
    internalRegisters[MDCR1] &= 0xFB00;
    internalRegisters[MDCR1] |= (value >> 16) & 0x3F;
    internalRegisters[MVSR1] = value;
}

void MCD212::SetDCP2(const uint32_t value)
{
    internalRegisters[MDDR2] &= 0x0F00;
    internalRegisters[MDDR2] |= (value >> 16) & 0x3F;
    internalRegisters[MDCP2] = value;
}

void MCD212::SetVSR2(const uint32_t value)
{
    internalRegisters[MDCR2] &= 0x0B00;
    internalRegisters[MDCR2] |= (value >> 16) & 0x3F;
    internalRegisters[MVSR2] = value;
}

void MCD212::ReloadDisplayParameters1(const bool CM, const uint8_t MF, const uint8_t FT)
{
    internalRegisters[MDCR1] &= 0xF33F;
    internalRegisters[MDCR1] |= CM << 11;
    internalRegisters[MDDR1] &= 0x003F;
    internalRegisters[MDDR1] |= MF << 10;
    internalRegisters[MDDR1] |= FT << 8;
}

void MCD212::ReloadDisplayParameters2(const bool CM, const uint8_t MF, const uint8_t FT)
{
    internalRegisters[MDCR2] &= 0x033F;
    internalRegisters[MDCR2] |= CM << 11;
    internalRegisters[MDDR2] &= 0x003F;
    internalRegisters[MDDR2] |= MF << 10;
    internalRegisters[MDDR2] |= FT << 8;
}

std::vector<VDSCRegister> MCD212::GetInternalRegisters() const
{
    std::vector<VDSCRegister> registers;
    registers.push_back(VDSCRegister({"CSR1R", MCSR1R + 0x4FFFE0, GetCSR1RRegister(), DisassembleCSR1RRegister()}));
    registers.push_back(VDSCRegister({"CSR2R", MCSR2R + 0x4FFFE0, GetCSR2RRegister(), DisassembleCSR2RRegister()}));
    registers.push_back(VDSCRegister({"CSR1W", MCSR1W + 0x4FFFE0, GetCSR1WRegister(), DisassembleCSR1WRegister()}));
    registers.push_back(VDSCRegister({"CSR2W", MCSR2W + 0x4FFFE0, GetCSR2WRegister(), DisassembleCSR2WRegister()}));
    registers.push_back(VDSCRegister({"DCR1",  MDCR1  + 0x4FFFE0,  GetDCR1Register(), DisassembleDCR1Register()}));
    registers.push_back(VDSCRegister({"DCR2",  MDCR2  + 0x4FFFE0,  GetDCR2Register(), DisassembleDCR2Register()}));
    registers.push_back(VDSCRegister({"DDR1",  MDDR1  + 0x4FFFE0,  GetDDR1Register(), DisassembleDDR1Register()}));
    registers.push_back(VDSCRegister({"DDR2",  MDDR2  + 0x4FFFE0,  GetDDR2Register(), DisassembleDDR2Register()}));
    registers.push_back(VDSCRegister({"VSR1",  MVSR1  + 0x4FFFE0,  GetVSR1Register(), DisassembleVSR1Register()}));
    registers.push_back(VDSCRegister({"VSR2",  MVSR2  + 0x4FFFE0,  GetVSR2Register(), DisassembleVSR2Register()}));
    registers.push_back(VDSCRegister({"DCP1",  MDCP1  + 0x4FFFE0,  GetDCP1Register(), DisassembleDCP1Register()}));
    registers.push_back(VDSCRegister({"DCP2",  MDCP2  + 0x4FFFE0,  GetDCP2Register(), DisassembleDCP2Register()}));
    return registers;
}

std::vector<VDSCRegister> MCD212::GetControlRegisters() const
{
    std::vector<VDSCRegister> registers;
    for(uint32_t i = 0; i < 256; i++)
        registers.push_back(VDSCRegister({"CLUT Color " + std::to_string(i), i, CLUT[i]}));
    registers.push_back(VDSCRegister({"Image Coding Method",              ImageCodingMethod          + 0x80, controlRegisters[ImageCodingMethod]}));
    registers.push_back(VDSCRegister({"Transparency Control",             TransparencyControl        + 0x80, controlRegisters[TransparencyControl]}));
    registers.push_back(VDSCRegister({"Plane Order",                      PlaneOrder                 + 0x80, controlRegisters[PlaneOrder]}));
    registers.push_back(VDSCRegister({"CLUT Bank",                        CLUTBank                   + 0x80, controlRegisters[CLUTBank]}));
    registers.push_back(VDSCRegister({"Transparent Color For Plane A",    TransparentColorForPlaneA  + 0x80, controlRegisters[TransparentColorForPlaneA]}));
    registers.push_back(VDSCRegister({"Transparent Color For Plane B",    TransparentColorForPlaneB  + 0x80, controlRegisters[TransparentColorForPlaneB]}));
    registers.push_back(VDSCRegister({"Mask Color For Plane A",           MaskColorForPlaneA         + 0x80, controlRegisters[MaskColorForPlaneA]}));
    registers.push_back(VDSCRegister({"Mask Color For Plane B",           MaskColorForPlaneB         + 0x80, controlRegisters[MaskColorForPlaneB]}));
    registers.push_back(VDSCRegister({"DYUV Abs Start Value For Plane A", DYUVAbsStartValueForPlaneA + 0x80, controlRegisters[DYUVAbsStartValueForPlaneA]}));
    registers.push_back(VDSCRegister({"DYUV Abs Start Value For Plane B", DYUVAbsStartValueForPlaneB + 0x80, controlRegisters[DYUVAbsStartValueForPlaneB]}));
    registers.push_back(VDSCRegister({"Cursor Position",                  CursorPosition             + 0x80, controlRegisters[CursorPosition]}));
    registers.push_back(VDSCRegister({"Cursor Control",                   CursorControl              + 0x80, controlRegisters[CursorControl]}));
    registers.push_back(VDSCRegister({"Cursor Pattern",                   CursorPattern              + 0x80, controlRegisters[CursorPattern]}));
    registers.push_back(VDSCRegister({"Region Control",                   RegionControl              + 0x80, controlRegisters[RegionControl]}));
    registers.push_back(VDSCRegister({"Backdrop Color",                   BackdropColor              + 0x80, controlRegisters[BackdropColor]}));
    registers.push_back(VDSCRegister({"Mosaic Pixel Hold For Plane A",    MosaicPixelHoldForPlaneA   + 0x80, controlRegisters[MosaicPixelHoldForPlaneA]}));
    registers.push_back(VDSCRegister({"Mosaic Pixel Hold For Plane B",    MosaicPixelHoldForPlaneB   + 0x80, controlRegisters[MosaicPixelHoldForPlaneB]}));
    registers.push_back(VDSCRegister({"Weight Factor For Plane A",        WeightFactorForPlaneA      + 0x80, controlRegisters[WeightFactorForPlaneA]}));
    registers.push_back(VDSCRegister({"Weight Factor For Plane B",        WeightFactorForPlaneB      + 0x80, controlRegisters[WeightFactorForPlaneB]}));
    return registers;
}

std::string MCD212::DisassembleCSR1RRegister() const
{
    return ("Display Active: " + std::string(GetDA() ? "1; " : "0; ")) + \
           ("Parity: " + std::string(GetSM() ? (GetPA() ? "odd" : "even") : "none"));
}

std::string MCD212::DisassembleCSR1WRegister() const
{
    return std::string(GetDI2() ? "Disable Interrupts 1: yes; " : "Disable Interrupts 1: no; ") + \
           ("DTACK Delay: " + std::string(GetDD() ? (std::to_string(3 + 2 * GetDD12()) + " >= " + std::to_string(4 + 2 * GetDD12()) + "; ") : "11 >= 12; ")) + \
           ("Type of DRAM: " + std::string(GetTD() ? "1M x 4; " : "256k x 4/16; ")) + \
           ("Standard Bit: " + std::string(GetST() ? "1; " : "0; ")) + \
           ("Bus Error: " + std::string(GetBE_W() ? "enabled;" : "disabled;"));
}

std::string MCD212::DisassembleDCR1Register() const
{
    return ("Display Enable: " + std::string(GetDE() ? "yes; " : "no; ")) + \
           ("Crystal Frequency: " + std::string(GetCF() ? "30, 30.2097 MHz; " : "28 MHz; ")) + \
           ("Frame Duration: " + std::string(GetFD() ? "60 Hz; " : "50 Hz; ")) + \
           ("Scan Mode: " + std::string(GetSM() ? "Interlace; " : "Non-interlace; ")) + \
           ("Bits per pixel plane A: " + std::string(GetCM1() ? "4; " : "8; ")) + \
           ("ICA1: " + std::string(GetIC1() ? "yes; " : "no; ")) + \
           ("DCA1: " + std::string(GetIC1() ? (GetDC1() ? "yes;" : "no;") : "no;"));
}

std::string MCD212::DisassembleVSR1Register() const
{
    return "0x" + toHex(GetVSR1());
}

std::string MCD212::DisassembleDDR1Register() const
{
    return ("Mosaic Factor plane A: " + std::string(GetFT12_1() == 3 ? (std::to_string(2 * pow(1, GetMF12_1())) + "; ") : "none; ")) + \
           ("File Type plane A: " + std::string(GetFT12_1() < 2 ? "Bitmap;" : (GetFT12_1() == 2 ? "Run-Length;" : "Mosaic;")));
}

std::string MCD212::DisassembleDCP1Register() const
{
    return "0x" + toHex(GetDCP1());
}

std::string MCD212::DisassembleCSR2RRegister() const
{
    return ("Interrupt 1: " + std::string(GetIT1() ? "yes; " : "no; ")) + \
           ("Interrupt 2: " + std::string(GetIT2() ? "yes; " : "no; ")) + \
           ("Bus Error: " + std::string(GetBE_R() ? "yes;" : "no;"));
}

std::string MCD212::DisassembleCSR2WRegister() const
{
    return GetDI2() ? "Disable Interrupts 2: yes;" : "Disable Interrupts 2: no;";
}

std::string MCD212::DisassembleDCR2Register() const
{
    return ("Bits per pixel plane B: " + std::string(GetCM2() ? "4; " : "8; ")) + \
           ("ICA2: " + std::string(GetIC2() ? "yes; " : "no; ")) + \
           ("DCA2: " + std::string(GetIC2() ? (GetDC2() ? "yes;" : "no;") : "no;"));
}

std::string MCD212::DisassembleVSR2Register() const
{
    return "0x" + toHex(GetVSR2());
}

std::string MCD212::DisassembleDDR2Register() const
{
    return ("Mosaic Factor plane B: " + std::string(GetFT12_2() == 3 ? (std::to_string(2 * pow(1, GetMF12_2())) + "; ") : "none; ")) + \
           ("File Type plane B: " + std::string(GetFT12_2() < 2 ? "Bitmap;" : (GetFT12_2() == 2 ? "Run-Length;" : "Mosaic;")));
}

std::string MCD212::DisassembleDCP2Register() const
{
    return "0x" + toHex(GetDCP2());
}
