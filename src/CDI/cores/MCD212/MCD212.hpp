#ifndef CDI_CORES_MCD212_MCD212_HPP
#define CDI_CORES_MCD212_MCD212_HPP

class CDI;
#include "../../common/utils.hpp"
#include "../../common/types.hpp"
#include "../../common/Video.hpp"
#include "../../OS9/BIOS.hpp"
#include "../../Video/Renderer.hpp"

#include <array>
#include <span>
#include <vector>

class MCD212
{
public:
    OS9::BIOS BIOS;
    uint32_t totalFrameCount{0};

    MCD212(CDI& idc, OS9::BIOS bios, bool pal);

    MCD212(const MCD212&) = delete;

    void Reset() noexcept;
    void IncrementTime(double ns);

    uint8_t  PeekByte(uint32_t addr) const noexcept;
    uint16_t PeekWord(uint32_t addr) const noexcept;

    uint8_t  GetByte(uint32_t addr, BusFlags flags);
    uint16_t GetWord(uint32_t addr, BusFlags flags);

    void SetByte(uint32_t addr, uint8_t  data, BusFlags flags);
    void SetWord(uint32_t addr, uint16_t data, BusFlags flags);

    RAMBank GetRAMBank1() const noexcept;
    RAMBank GetRAMBank2() const noexcept;

    std::vector<InternalRegister> GetInternalRegisters() const;
    std::vector<InternalRegister> GetControlRegisters() const;
    const Video::Plane& GetScreen() const noexcept { return renderer.m_screen; }
    const Video::Plane& GetPlaneA() const noexcept { return renderer.m_plane[Video::Renderer::A]; }
    const Video::Plane& GetPlaneB() const noexcept { return renderer.m_plane[Video::Renderer::B]; }
    const Video::Plane& GetBackground() const noexcept { return renderer.m_backdropPlane; }
    const Video::Plane& GetCursor() const noexcept { return renderer.m_cursorPlane; }

private:
    CDI& cdi;
    const bool isPAL;
    uint8_t memorySwapCount{0};
    double timeNs{0.0}; // time counter in nano seconds.

    static constexpr Video::Renderer::ImagePlane PlaneA = Video::Renderer::A;
    static constexpr Video::Renderer::ImagePlane PlaneB = Video::Renderer::B;

    Video::Renderer renderer{};

    std::vector<uint8_t> memory;

    std::array<uint16_t, 32> internalRegisters{0};
    uint8_t registerCSR1R{0};
    uint8_t registerCSR2R{0};

    uint16_t verticalLines{0}; // starts at 0.

    void DrawVideoLine();

    // Control Area
    // TODO: template this against Video::Renderer::Plane ?
    void ExecuteICA1();
    void ExecuteDCA1();
    void ExecuteICA2();
    void ExecuteDCA2();

    void ResetMemorySwap() noexcept;
    uint32_t GetControlInstruction(uint32_t addr);

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
    bool GetDA() const noexcept { return bit<7>(registerCSR1R); }
    bool GetPA() const noexcept { return bit<5>(registerCSR1R); }
    // CSR2R
    bool GetIT1() const noexcept { return bit<2>(registerCSR2R); }
    bool GetIT2() const noexcept { return bit<1>(registerCSR2R); }
    bool GetBE_R() const noexcept { return bit<0>(registerCSR2R); }

    // CSR1W
    bool GetDI1() const noexcept { return bit<15>(internalRegisters[CSR1W]); }
    uint8_t GetDD12() const noexcept { return bits<8, 9>(internalRegisters[CSR1W]); }
    bool GetTD() const noexcept { return bit<5>(internalRegisters[CSR1W]); }
    bool GetDD() const noexcept { return bit<3>(internalRegisters[CSR1W]); }
    bool GetST() const noexcept { return bit<1>(internalRegisters[CSR1W]); }
    bool GetBE_W() const noexcept { return bit<0>(internalRegisters[CSR1W]); }
    // CSR2W
    bool GetDI2() const noexcept { return bit<15>(internalRegisters[CSR2W]); }

    // DCR1
    bool GetDE() const noexcept { return bit<15>(internalRegisters[DCR1]); }
    bool GetCF() const noexcept { return bit<14>(internalRegisters[DCR1]); }
    bool GetFD() const noexcept { return bit<13>(internalRegisters[DCR1]); }
    bool GetSM() const noexcept { return bit<12>(internalRegisters[DCR1]); }
    bool GetCM1() const noexcept { return bit<11>(internalRegisters[DCR1]); }
    bool GetIC1() const noexcept { return bit<9>(internalRegisters[DCR1]); }
    bool GetDC1() const noexcept { return bit<8>(internalRegisters[DCR1]); }
    // DCR2
    bool GetCM2() const noexcept { return bit<11>(internalRegisters[DCR2]); }
    bool GetIC2() const noexcept { return bit<9>(internalRegisters[DCR2]); }
    bool GetDC2() const noexcept { return bit<8>(internalRegisters[DCR2]); }

    // DDR1
    uint8_t GetMF12_1() const noexcept { return bits<10, 11>(internalRegisters[DDR1]); }
    uint8_t GetFT12_1() const noexcept { return bits<8, 9>(internalRegisters[DDR1]); }
    // DDR2
    uint8_t GetMF12_2() const noexcept { return bits<10, 11>(internalRegisters[DDR2]); }
    uint8_t GetFT12_2() const noexcept { return bits<8, 9>(internalRegisters[DDR2]); }

    uint32_t GetVSR1() const noexcept { return as<uint32_t>(bits<0, 5>(internalRegisters[DCR1])) << 16 | internalRegisters[VSR1]; }
    uint32_t GetVSR2() const noexcept { return as<uint32_t>(bits<0, 5>(internalRegisters[DCR2])) << 16 | internalRegisters[VSR2]; }
    uint32_t GetDCP1() const noexcept { return as<uint32_t>(bits<0, 5>(internalRegisters[DDR1])) << 16 | internalRegisters[DCP1]; }
    uint32_t GetDCP2() const noexcept { return as<uint32_t>(bits<0, 5>(internalRegisters[DDR2])) << 16 | internalRegisters[DCP2]; }

    void SetIT1(bool it = true);
    void SetIT2(bool it = true);
    void SetDCP1(uint32_t value);
    void SetVSR1(uint32_t value);
    void SetDCP2(uint32_t value);
    void SetVSR2(uint32_t value);
    void ReloadDisplayParameters1(bool dm, uint8_t MF, uint8_t FT);
    void ReloadDisplayParameters2(bool dm, uint8_t MF, uint8_t FT);

    uint16_t GetHorizontalResolution1() const noexcept { uint16_t a = GetCF() ? (GetST() ? 360 : 384) : 360; return GetCM1() ? a*2 : a; }
    uint16_t GetHorizontalResolution2() const noexcept { uint16_t a = GetCF() ? (GetST() ? 360 : 384) : 360; return GetCM2() ? a*2 : a; }
    uint16_t GetVerticalResolution() const noexcept { return GetFD() ? 240 : (GetST() ? 240 : 280); }

    size_t GetHorizontalCycles() const noexcept { return GetCF() ? 120 : 112; } // Table 5.5
    size_t GetTotalVerticalLines() const noexcept { return GetFD() ? 262 : 312; } // Table 5.6
    size_t GetVerticalRetraceLines() const noexcept { return GetFD() ? 22 : (GetST() ? 72 : 32); }

    size_t GetLineDisplayTime() const noexcept // as nano seconds
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
