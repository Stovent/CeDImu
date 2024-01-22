#ifndef CDI_CORES_MCD212_MCD212_HPP
#define CDI_CORES_MCD212_MCD212_HPP

class CDI;
#include "../../common/utils.hpp"
#include "../../common/types.hpp"
#include "../../common/Video.hpp"
#include "../../OS9/BIOS.hpp"

#include <array>
#include <span>
#include <vector>

class MCD212
{
public:
    OS9::BIOS BIOS;
    uint32_t totalFrameCount;

    MCD212(CDI& idc, std::span<const uint8_t> systemBios, const bool PAL);

    MCD212(const MCD212&) = delete;

    void Reset();
    void IncrementTime(const double ns);

    uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Log | Trigger);
    uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger);

    void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Log | Trigger);
    void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger);

    RAMBank GetRAMBank1() const;
    RAMBank GetRAMBank2() const;

    std::vector<InternalRegister> GetInternalRegisters() const;
    std::vector<InternalRegister> GetControlRegisters() const;
    const Video::Plane& GetScreen() const;
    const Video::Plane& GetPlaneA() const;
    const Video::Plane& GetPlaneB() const;
    const Video::Plane& GetBackground() const;
    const Video::Plane& GetCursor() const;

private:
    CDI& cdi;
    const bool isPAL;
    uint8_t memorySwapCount;
    double timeNs; // time counter in nano seconds.

    std::vector<uint8_t> memory;

    Video::Plane screen;
    Video::Plane planeA;
    Video::Plane planeB;
    Video::Plane backgroundPlane;
    Video::Plane cursorPlane;

    std::array<uint32_t, 0x80> controlRegisters;
    std::array<uint32_t, 256> CLUT;
    std::array<uint16_t, 16> cursorPatterns;
    std::array<bool, 2> regionFlags;
    uint8_t currentRegionControl;

    std::array<uint16_t, 32> internalRegisters;
    uint8_t registerCSR1R;
    uint8_t registerCSR2R;

    uint16_t lineNumber; // starts at 0
    uint16_t verticalLines; // starts at 0.

    void ExecuteVideoLine();
    void DrawLinePlaneA();
    void DrawLinePlaneB();
    void DrawLineBackground();
    void DrawLineCursor();
    void OverlayMix();
    void HandleTransparency(uint8_t* pixel, const uint32_t control, const uint32_t color);
    void HandleRegions(const uint16_t pos);

    // Display File Decoders
    void DecodeMosaicLineA();
    void DecodeMosaicLineB();

    // Control Area
    void ExecuteICA1();
    void ExecuteDCA1();
    void ExecuteICA2();
    void ExecuteDCA2();

    void MemorySwap();
    uint32_t GetLong(const uint32_t addr);

    // internal registers
    std::string DisassembleCSR1RRegister() const;
    std::string DisassembleCSR1WRegister() const;
    std::string DisassembleDCR1Register() const;
    std::string DisassembleVSR1Register() const;
    std::string DisassembleDDR1Register() const;
    std::string DisassembleDCP1Register() const;

    std::string DisassembleCSR2RRegister() const;
    std::string DisassembleCSR2WRegister() const;
    std::string DisassembleDCR2Register() const;
    std::string DisassembleVSR2Register() const;
    std::string DisassembleDDR2Register() const;
    std::string DisassembleDCP2Register() const;

    // CSR1R
    bool GetDA() const { return bit<7>(registerCSR1R); }
    bool GetPA() const { return bit<5>(registerCSR1R); }
    // CSR2R
    bool GetIT1() const { return bit<2>(registerCSR2R); }
    bool GetIT2() const { return bit<1>(registerCSR2R); }
    bool GetBE_R() const { return bit<0>(registerCSR2R); }

    // CSR1W
    bool GetDI1() const { return bit<15>(internalRegisters[CSR1W]); }
    uint8_t GetDD12() const { return bits<8, 9>(internalRegisters[CSR1W]); }
    bool GetTD() const { return bit<5>(internalRegisters[CSR1W]); }
    bool GetDD() const { return bit<3>(internalRegisters[CSR1W]); }
    bool GetST() const { return bit<1>(internalRegisters[CSR1W]); }
    bool GetBE_W() const { return bit<0>(internalRegisters[CSR1W]); }
    // CSR2W
    bool GetDI2() const { return bit<15>(internalRegisters[CSR2W]); }

    // DCR1
    bool GetDE() const { return bit<15>(internalRegisters[DCR1]); }
    bool GetCF() const { return bit<14>(internalRegisters[DCR1]); }
    bool GetFD() const { return bit<13>(internalRegisters[DCR1]); }
    bool GetSM() const { return bit<12>(internalRegisters[DCR1]); }
    bool GetCM1() const { return bit<11>(internalRegisters[DCR1]); }
    bool GetIC1() const { return bit<9>(internalRegisters[DCR1]); }
    bool GetDC1() const { return bit<8>(internalRegisters[DCR1]); }
    // DCR2
    bool GetCM2() const { return bit<11>(internalRegisters[DCR2]); }
    bool GetIC2() const { return bit<9>(internalRegisters[DCR2]); }
    bool GetDC2() const { return bit<8>(internalRegisters[DCR2]); }

    // DDR1
    uint8_t GetMF12_1() const { return bits<10, 11>(internalRegisters[DDR1]); }
    uint8_t GetFT12_1() const { return bits<8, 9>(internalRegisters[DDR1]); }
    // DDR2
    uint8_t GetMF12_2() const { return bits<10, 11>(internalRegisters[DDR2]); }
    uint8_t GetFT12_2() const { return bits<8, 9>(internalRegisters[DDR2]); }

    uint32_t GetVSR1() const { return as<uint32_t>(bits<0, 5>(internalRegisters[DCR1])) << 16 | internalRegisters[VSR1]; }
    uint32_t GetVSR2() const { return as<uint32_t>(bits<0, 5>(internalRegisters[DCR2])) << 16 | internalRegisters[VSR2]; }
    uint32_t GetDCP1() const { return as<uint32_t>(bits<0, 5>(internalRegisters[DDR1])) << 16 | internalRegisters[DCP1]; }
    uint32_t GetDCP2() const { return as<uint32_t>(bits<0, 5>(internalRegisters[DDR2])) << 16 | internalRegisters[DCP2]; }

    void SetIT1(const bool it = true);
    void SetIT2(const bool it = true);
    void SetDCP1(const uint32_t value);
    void SetVSR1(const uint32_t value);
    void SetDCP2(const uint32_t value);
    void SetVSR2(const uint32_t value);
    void ReloadDisplayParameters1(const bool dm, const uint8_t MF, const uint8_t FT);
    void ReloadDisplayParameters2(const bool dm, const uint8_t MF, const uint8_t FT);

    uint16_t GetHorizontalResolution1() const { uint16_t a = GetCF() ? (GetST() ? 360 : 384) : 360; return GetCM1() ? a*2 : a; }
    uint16_t GetHorizontalResolution2() const { uint16_t a = GetCF() ? (GetST() ? 360 : 384) : 360; return GetCM2() ? a*2 : a; }
    uint16_t GetVerticalResolution() const { return GetFD() ? 240 : (GetST() ? 240 : 280); }

    size_t GetHorizontalCycles() const { return GetCF() ? 120 : 112; } // Table 5.5
    size_t GetTotalVerticalLines() const { return GetFD() ? 262 : 312; } // Table 5.6
    size_t GetVerticalRetraceLines() const { return GetFD() ? 22 : (GetST() ? 72 : 32); }

    size_t GetLineDisplayTime() const // as nano seconds
    {
        return isPAL || !GetCF() ? 64000 : 63560;
    }

    enum InternalRegistersMemoryMap
    {
        CSR2W = 0x00,
        CSR2R = 0x01,
        DCR2  = 0x02,
        VSR2  = 0x04,
        DDR2  = 0x08,
        DCP2  = 0x0A,
        CSR1W = 0x10,
        CSR1R = 0x11,
        DCR1  = 0x12,
        VSR1  = 0x14,
        DDR1  = 0x18,
        DCP1  = 0x1A,
    };

    enum ControlRegister
    {
        CLUTColor = 0x00,
        ImageCodingMethod = 0x40,
        TransparencyControl,
        PlaneOrder,
        CLUTBank,
        TransparentColorForPlaneA,
        reserved1,
        TransparentColorForPlaneB,
        MaskColorForPlaneA,
        reserved2,
        MaskColorForPlaneB,
        DYUVAbsStartValueForPlaneA,
        DYUVAbsStartValueForPlaneB,
        reserved3,
        CursorPosition,
        CursorControl,
        CursorPattern,
        RegionControl,
        BackdropColor = 0x58,
        MosaicPixelHoldForPlaneA,
        MosaicPixelHoldForPlaneB,
        WeightFactorForPlaneA,
        WeightFactorForPlaneB,
    };
};

#endif // CDI_CORES_MCD212_MCD212_HPP
