#ifndef CDI_CORES_MCD212_MCD212_HPP
#define CDI_CORES_MCD212_MCD212_HPP

class CDI;
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
    Video::Plane cursorPlane;
    Video::Plane backgroundPlane;

    std::array<uint32_t, 0x80> controlRegisters;
    std::array<uint32_t, 256> CLUT;
    std::array<uint16_t, 16>  cursorPatterns;
    std::array<std::array<bool, Video::Plane::MAX_WIDTH>, 2> regionFlags;
    uint8_t currentRegionControl;

    std::array<uint16_t, 32> internalRegisters;
    uint8_t registerCSR1R;
    uint8_t registerCSR2R;

    uint16_t lineNumber; // starts at 0
    uint16_t verticalLines; // starts at 0.

    void ExecuteVideoLine();
    void DrawLinePlaneA();
    void DrawLinePlaneB();
    void DrawLineCursor();
    void DrawLineBackground();
    void OverlayMix();
    void HandleTransparency(uint8_t* pixel, const uint16_t pos, const uint32_t control, const uint32_t color);
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
    uint16_t GetCSR1RRegister() const;
    uint16_t GetCSR1WRegister() const;
    uint16_t GetDCR1Register() const;
    uint16_t GetVSR1Register() const;
    uint16_t GetDDR1Register() const;
    uint16_t GetDCP1Register() const;

    uint16_t GetCSR2RRegister() const;
    uint16_t GetCSR2WRegister() const;
    uint16_t GetDCR2Register() const;
    uint16_t GetVSR2Register() const;
    uint16_t GetDDR2Register() const;
    uint16_t GetDCP2Register() const;

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

    bool GetDA() const;
    bool GetPA() const;
    bool GetIT1() const;
    bool GetIT2() const;
    bool GetBE_R() const;
    bool GetBE_W() const;
    bool GetDI1() const;
    uint8_t GetDD12() const;
    bool GetTD() const;
    bool GetDD() const;
    bool GetST() const;
    bool GetDI2() const;
    bool GetDE() const;
    bool GetCF() const;
    bool GetFD() const;
    bool GetSM() const;
    bool GetCM1() const;
    bool GetIC1() const;
    bool GetDC1() const;
    bool GetCM2() const;
    bool GetIC2() const;
    bool GetDC2() const;
    uint8_t GetMF12_1() const;
    uint8_t GetFT12_1() const;
    uint8_t GetMF12_2() const;
    uint8_t GetFT12_2() const;
    uint32_t GetVSR1() const;
    uint32_t GetVSR2() const;
    uint32_t GetDCP1() const;
    uint32_t GetDCP2() const;

    void SetIT1(const bool it = true);
    void SetIT2(const bool it = true);
    void SetDCP1(const uint32_t value);
    void SetVSR1(const uint32_t value);
    void SetDCP2(const uint32_t value);
    void SetVSR2(const uint32_t value);
    void ReloadDisplayParameters1(const bool dm, const uint8_t MF, const uint8_t FT);
    void ReloadDisplayParameters2(const bool dm, const uint8_t MF, const uint8_t FT);

    inline uint16_t GetHorizontalResolution1() const { uint16_t a = GetCF() ? (GetST() ? 360 : 384) : 360; return GetCM1() ? a*2 : a; }
    inline uint16_t GetHorizontalResolution2() const { uint16_t a = GetCF() ? (GetST() ? 360 : 384) : 360; return GetCM2() ? a*2 : a; }
    inline uint16_t GetVerticalResolution() const { return GetFD() ? 240 : (GetST() ? 240 : 280); }

    inline size_t GetHorizontalCycles() const { return GetCF() ? 120 : 112; } // Table 5.5
    inline size_t GetTotalVerticalLines() const { return GetFD() ? 262 : 312; } // Table 5.6
    inline size_t GetVerticalRetraceLines() const { return GetFD() ? 22 : (GetST() ? 72 : 32); }

    inline size_t GetLineDisplayTime() const // as nano seconds
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
