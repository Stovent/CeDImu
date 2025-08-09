#ifndef CDI_CORES_MCD212_MCD212_HPP
#define CDI_CORES_MCD212_MCD212_HPP

class CDI;
#include "common/utils.hpp"
#include "common/types.hpp"
#include "OS9/BIOS.hpp"
#ifdef LIBCEDIMU_ENABLE_RENDERERSIMD
#include "Video/RendererSIMD.hpp"
#endif
#include "Video/RendererSoftware.hpp"

#include <array>
#include <span>
#include <vector>

class MCD212
{
public:
    // TODO: NTSC runs at 30.2098 MHz.

    static constexpr Video::Renderer::ImagePlane PlaneA = Video::Renderer::A;
    static constexpr Video::Renderer::ImagePlane PlaneB = Video::Renderer::B;

    OS9::BIOS m_bios;
    uint32_t m_totalFrameCount{0};

    MCD212(CDI& cdi, OS9::BIOS bios, bool pal);

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
    const Video::Plane& GetScreen() const noexcept { return m_renderer.m_screen; }
    const Video::Plane& GetPlaneA() const noexcept { return m_renderer.m_plane[PlaneA]; }
    const Video::Plane& GetPlaneB() const noexcept { return m_renderer.m_plane[PlaneB]; }
    const Video::Plane& GetBackground() const noexcept { return m_renderer.m_backdropPlane; }
    const Video::Plane& GetCursor() const noexcept { return m_renderer.m_cursorPlane; }

private:
    CDI& m_cdi;
    const bool m_isPAL;
    uint8_t m_memorySwapCount{0};
    double m_timeNs{0.0}; // time counter in nano seconds.

#ifdef LIBCEDIMU_ENABLE_RENDERERSIMD
    Video::RendererSIMD m_renderer{};
#else
    Video::RendererSoftware m_renderer{};
#endif

    std::vector<uint8_t> m_memory;

    std::array<uint16_t, 32> m_internalRegisters{0};
    uint8_t m_registerCSR1R{0};
    uint8_t m_registerCSR2R{0};

    uint16_t m_verticalLines{0}; // starts at 0.
    uint16_t m_lineNumber{0}; // starts at 0.

    void DrawVideoLine();

    // Control Area
    template<Video::Renderer::ImagePlane PLANE>
    void ExecuteICA();
    template<Video::Renderer::ImagePlane PLANE>
    void ExecuteDCA();

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
    bool GetDA() const noexcept { return bit<7>(m_registerCSR1R); }
    void SetDA() noexcept { m_registerCSR1R |= 0x80; }
    void UnsetDA() noexcept { m_registerCSR1R &= 0x20; }
    bool GetPA() const noexcept { return bit<5>(m_registerCSR1R); }
    void SetPA() noexcept { m_registerCSR1R |= 0x20; }
    void UnsetPA() noexcept { m_registerCSR1R &= 0x80; }
    // CSR2R
    bool GetIT1() const noexcept { return bit<2>(m_registerCSR2R); }
    bool GetIT2() const noexcept { return bit<1>(m_registerCSR2R); }
    bool GetBE_R() const noexcept { return bit<0>(m_registerCSR2R); }

    // CSR1W
    bool GetDI1() const noexcept { return bit<15>(m_internalRegisters[CSR1W]); }
    uint8_t GetDD12() const noexcept { return bits<8, 9>(m_internalRegisters[CSR1W]); }
    bool GetTD() const noexcept { return bit<5>(m_internalRegisters[CSR1W]); }
    bool GetDD() const noexcept { return bit<3>(m_internalRegisters[CSR1W]); }
    bool GetST() const noexcept { return bit<1>(m_internalRegisters[CSR1W]); }
    bool GetBE_W() const noexcept { return bit<0>(m_internalRegisters[CSR1W]); }
    // CSR2W
    bool GetDI2() const noexcept { return bit<15>(m_internalRegisters[CSR2W]); }

    // DCR1
    bool GetDE() const noexcept { return bit<15>(m_internalRegisters[DCR1]); }
    bool GetCF() const noexcept { return bit<14>(m_internalRegisters[DCR1]); }
    bool GetFD() const noexcept { return bit<13>(m_internalRegisters[DCR1]); }
    bool GetSM() const noexcept { return bit<12>(m_internalRegisters[DCR1]); }
    bool GetCM1() const noexcept { return bit<11>(m_internalRegisters[DCR1]); }
    bool GetIC1() const noexcept { return bit<9>(m_internalRegisters[DCR1]); }
    bool GetDC1() const noexcept { return bit<8>(m_internalRegisters[DCR1]); }
    // DCR2
    bool GetCM2() const noexcept { return bit<11>(m_internalRegisters[DCR2]); }
    bool GetIC2() const noexcept { return bit<9>(m_internalRegisters[DCR2]); }
    bool GetDC2() const noexcept { return bit<8>(m_internalRegisters[DCR2]); }

    // DDR1
    uint8_t GetMF12_1() const noexcept { return bits<10, 11>(m_internalRegisters[DDR1]); }
    uint8_t GetFT12_1() const noexcept { return bits<8, 9>(m_internalRegisters[DDR1]); }
    // DDR2
    uint8_t GetMF12_2() const noexcept { return bits<10, 11>(m_internalRegisters[DDR2]); }
    uint8_t GetFT12_2() const noexcept { return bits<8, 9>(m_internalRegisters[DDR2]); }

    uint32_t GetVSR1() const noexcept { return as<uint32_t>(bits<0, 5>(m_internalRegisters[DCR1])) << 16 | m_internalRegisters[VSR1]; }
    uint32_t GetVSR2() const noexcept { return as<uint32_t>(bits<0, 5>(m_internalRegisters[DCR2])) << 16 | m_internalRegisters[VSR2]; }
    uint32_t GetDCP1() const noexcept { return as<uint32_t>(bits<0, 5>(m_internalRegisters[DDR1])) << 16 | m_internalRegisters[DCP1]; }
    uint32_t GetDCP2() const noexcept { return as<uint32_t>(bits<0, 5>(m_internalRegisters[DDR2])) << 16 | m_internalRegisters[DCP2]; }

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
        return m_isPAL || !GetCF() ? 64000 : 63560;
    }

    bool Is60FPS() const noexcept
    {
        return GetFD();
    }

    Video::Renderer::DisplayFormat GetDisplayFormat() const noexcept
    {
        using enum Video::Renderer::DisplayFormat;
        if(!GetCF() && !GetST())
            return NTSCMonitor; // 5.2.1

        if(GetFD())
            return NTSCTV;
        else
            return PAL;
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
