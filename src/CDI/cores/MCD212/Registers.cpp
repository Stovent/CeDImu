#include "MCD212.hpp"
#include "../../common/utils.hpp"

void MCD212::SetIT1(const bool it)
{
    m_registerCSR2R &= 0x0003;
    m_registerCSR2R |= it << 2;
}

void MCD212::SetIT2(const bool it)
{
    m_registerCSR2R &= 0x0005;
    m_registerCSR2R |= it << 1;
}

void MCD212::SetDCP1(const uint32_t value)
{
    m_internalRegisters[DDR1] &= 0x0F00;
    m_internalRegisters[DDR1] |= bits<16, 21>(value);
    m_internalRegisters[DCP1] = value;
}

void MCD212::SetVSR1(const uint32_t value)
{
    m_internalRegisters[DCR1] &= 0xFB00;
    m_internalRegisters[DCR1] |= bits<16, 21>(value);
    m_internalRegisters[VSR1] = value;
}

void MCD212::SetDCP2(const uint32_t value)
{
    m_internalRegisters[DDR2] &= 0x0F00;
    m_internalRegisters[DDR2] |= bits<16, 21>(value);
    m_internalRegisters[DCP2] = value;
}

void MCD212::SetVSR2(const uint32_t value)
{
    m_internalRegisters[DCR2] &= 0x0B00;
    m_internalRegisters[DCR2] |= bits<16, 21>(value);
    m_internalRegisters[VSR2] = value;
}

void MCD212::ReloadDisplayParameters1(const bool CM, const uint8_t MF, const uint8_t FT)
{
    m_internalRegisters[DCR1] &= 0xF33F;
    m_internalRegisters[DCR1] |= CM << 11;
    m_internalRegisters[DDR1] &= 0x003F;
    m_internalRegisters[DDR1] |= MF << 10;
    m_internalRegisters[DDR1] |= FT << 8;
}

void MCD212::ReloadDisplayParameters2(const bool CM, const uint8_t MF, const uint8_t FT)
{
    m_internalRegisters[DCR2] &= 0x033F;
    m_internalRegisters[DCR2] |= CM << 11;
    m_internalRegisters[DDR2] &= 0x003F;
    m_internalRegisters[DDR2] |= MF << 10;
    m_internalRegisters[DDR2] |= FT << 8;
}

std::vector<InternalRegister> MCD212::GetInternalRegisters() const
{
    return {
        {"CSR1R", CSR1R + 0x4FFFE0, m_registerCSR1R, DisassembleCSR1RRegister()},
        {"CSR2R", CSR2R + 0x4FFFE0, m_registerCSR2R, DisassembleCSR2RRegister()},
        {"CSR1W", CSR1W + 0x4FFFE0, m_internalRegisters[CSR1W], DisassembleCSR1WRegister()},
        {"CSR2W", CSR2W + 0x4FFFE0, m_internalRegisters[CSR2W], DisassembleCSR2WRegister()},
        {"DCR1",  DCR1  + 0x4FFFE0, m_internalRegisters[DCR1],  DisassembleDCR1Register()},
        {"DCR2",  DCR2  + 0x4FFFE0, m_internalRegisters[DCR2],  DisassembleDCR2Register()},
        {"DDR1",  DDR1  + 0x4FFFE0, m_internalRegisters[DDR1],  DisassembleDDR1Register()},
        {"DDR2",  DDR2  + 0x4FFFE0, m_internalRegisters[DDR2],  DisassembleDDR2Register()},
        {"VSR1",  VSR1  + 0x4FFFE0, m_internalRegisters[VSR1],  DisassembleVSR1Register()},
        {"VSR2",  VSR2  + 0x4FFFE0, m_internalRegisters[VSR2],  DisassembleVSR2Register()},
        {"DCP1",  DCP1  + 0x4FFFE0, m_internalRegisters[DCP1],  DisassembleDCP1Register()},
        {"DCP2",  DCP2  + 0x4FFFE0, m_internalRegisters[DCP2],  DisassembleDCP2Register()},
    };
}

std::vector<InternalRegister> MCD212::GetControlRegisters() const
{
    std::vector<InternalRegister> registers;

    for(uint32_t i = 0; i < 256; i++)
        registers.emplace_back("CLUT Color " + std::to_string(i), i, m_renderer.m_clut[i], "");

    registers.emplace_back("Image Coding Method Plane A",       ImageCodingMethod          + 0x80, as<int>(m_renderer.m_codingMethod[PlaneA]), "");
    registers.emplace_back("Image Coding Method Plane B",       ImageCodingMethod          + 0x80, as<int>(m_renderer.m_codingMethod[PlaneB]), "");
    registers.emplace_back("External Video",                    ImageCodingMethod          + 0x80, m_renderer.m_externalVideo, "");
    registers.emplace_back("Number of regions",                 ImageCodingMethod          + 0x80, m_renderer.m_matteNumber, "");
    registers.emplace_back("CLUT Select",                       ImageCodingMethod          + 0x80, m_renderer.m_clutSelectHigh, "");
    registers.emplace_back("Transparency Control Plane A",      TransparencyControl        + 0x80, m_renderer.m_transparencyControl[PlaneA], "");
    registers.emplace_back("Transparency Control Plane B",      TransparencyControl        + 0x80, m_renderer.m_transparencyControl[PlaneB], "");
    registers.emplace_back("Mixing",                            TransparencyControl        + 0x80, m_renderer.m_mix, "");
    registers.emplace_back("Plane Order",                       PlaneOrder                 + 0x80, m_renderer.m_planeOrder, "");
    registers.emplace_back("CLUT Bank",                         CLUTBank                   + 0x80, m_renderer.m_clutBank, "");
    registers.emplace_back("Transparent Color For Plane A",     TransparentColorForPlaneA  + 0x80, m_renderer.m_transparentColorRgb[PlaneA], "");
    registers.emplace_back("Transparent Color For Plane B",     TransparentColorForPlaneB  + 0x80, m_renderer.m_transparentColorRgb[PlaneB], "");
    registers.emplace_back("Mask Color For Plane A",            MaskColorForPlaneA         + 0x80, m_renderer.m_maskColorRgb[PlaneA], "");
    registers.emplace_back("Mask Color For Plane B",            MaskColorForPlaneB         + 0x80, m_renderer.m_maskColorRgb[PlaneB], "");
    registers.emplace_back("DYUV Abs Start Value For Plane A",  DYUVAbsStartValueForPlaneA + 0x80, m_renderer.m_dyuvInitialValue[PlaneA], "");
    registers.emplace_back("DYUV Abs Start Value For Plane B",  DYUVAbsStartValueForPlaneB + 0x80, m_renderer.m_dyuvInitialValue[PlaneB], "");
    registers.emplace_back("Cursor X Position",                 CursorPosition             + 0x80, m_renderer.m_cursorX, "");
    registers.emplace_back("Cursor Y Position",                 CursorPosition             + 0x80, m_renderer.m_cursorY, "");
    registers.emplace_back("Cursor Enabled",                    CursorControl              + 0x80, m_renderer.m_cursorEnabled, "");
    registers.emplace_back("Cursor color",                      CursorControl              + 0x80, m_renderer.m_cursorColor, "");

    for(size_t i = 0; i < m_renderer.m_cursorPatterns.size(); i++)
        registers.emplace_back("Cursor Pattern " + std::to_string(i), CursorPattern + 0x80, m_renderer.m_cursorPatterns[i], "");
    for(size_t i = 0; i < m_renderer.m_matteControl.size(); i++)
        registers.emplace_back("Region Control " + std::to_string(i), RegionControl + 0x80 + i, m_renderer.m_matteControl[i], "");

    registers.emplace_back("Backdrop Color",                    BackdropColor              + 0x80, m_renderer.m_backdropColor, "");
    registers.emplace_back("Mosaic Pixel Hold enabled Plane A", MosaicPixelHoldForPlaneA   + 0x80, m_renderer.m_holdEnabled[PlaneA], "");
    registers.emplace_back("Mosaic Pixel Hold factor Plane A",  MosaicPixelHoldForPlaneA   + 0x80, m_renderer.m_holdFactor[PlaneA], "");
    registers.emplace_back("Mosaic Pixel Hold enabled Plane B", MosaicPixelHoldForPlaneB   + 0x80, m_renderer.m_holdEnabled[PlaneB], "");
    registers.emplace_back("Mosaic Pixel Hold factor Plane B",  MosaicPixelHoldForPlaneB   + 0x80, m_renderer.m_holdFactor[PlaneB], "");
    registers.emplace_back("Weight Factor For Plane A",         WeightFactorForPlaneA      + 0x80, m_renderer.m_icf[PlaneA], "");
    registers.emplace_back("Weight Factor For Plane B",         WeightFactorForPlaneB      + 0x80, m_renderer.m_icf[PlaneB], "");

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
    return ("Mosaic Factor plane A: " + std::string(GetFT12_1() == 3 ? (std::to_string(1 << (GetMF12_1() + 1)) + "; ") : "none; ")) + \
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
    return ("Mosaic Factor plane B: " + std::string(GetFT12_2() == 3 ? (std::to_string(1 << (GetMF12_2() + 1)) + "; ") : "none; ")) + \
           ("File Type plane B: " + std::string(GetFT12_2() < 2 ? "Bitmap;" : (GetFT12_2() == 2 ? "Run-Length;" : "Mosaic;")));
}

std::string MCD212::DisassembleDCP2Register() const
{
    return "0x" + toHex(GetDCP2());
}
