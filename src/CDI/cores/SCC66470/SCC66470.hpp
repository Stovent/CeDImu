#ifndef CDI_CORES_SCC66470_SCC66470_HPP
#define CDI_CORES_SCC66470_SCC66470_HPP

class CDI;
#include "../../common/types.hpp"
#include "../../common/Video.hpp"
#include "../../OS9/BIOS.hpp"

#include <array>
#include <vector>

class SCC66470
{
public:
    CDI& cdi;
    OS9::BIOS BIOS;
    uint32_t totalFrameCount;

    SCC66470(CDI& idc, const bool ismaster, const void* bios, const uint32_t size);

    SCC66470(const SCC66470&) = delete;

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
    const bool isMaster;
    std::vector<uint8_t> memory;
    uint8_t memorySwapCount;
    uint16_t lineNumber; // starts at 0

    std::array<uint16_t, 0x20> internalRegisters;
    uint8_t registerCSR;
    uint16_t registerB;

    Video::Plane screen;
    Video::Plane planeA;
    Video::Plane planeB;
    Video::Plane cursorPlane;
    Video::Plane backgroundPlane;

    void MemorySwap();
    uint32_t GetLong(const uint32_t addr, const uint8_t flags = Trigger);

    void ExecuteVideoLine();

    // Internal Register
    uint16_t GetCSRWRegister() const;
    uint16_t GetCSRRRegister() const;
    uint16_t GetDCRRegister() const;
    uint16_t GetVSRRegister() const;
    uint16_t GetBCRRegister() const;
    uint16_t GetDCR2Register() const;
    uint16_t GetDCPRegister() const;
    uint16_t GetSWMRegister() const;
    uint16_t GetSTMRegister() const;
    uint16_t GetARegister() const;
    uint16_t GetBRegister() const;
    uint16_t GetPCRRegister() const;
    uint16_t GetMASKRegister() const;
    uint16_t GetSHIFTRegister() const;
    uint16_t GetINDEXRegister() const;
    uint16_t GetFCRegister() const;
    uint16_t GetBCRegister() const;
    uint16_t GetTCRegister() const;

    bool GetFD() const;
    bool GetSS() const;
    bool GetST() const;

    inline uint16_t GetHorizontalResolution() const { return 0; }
    inline uint16_t GetVerticalResolution() const { return GetFD() ? (GetSS() ? 240 : 210) : (GetSS() ? (GetST() ? 240 : 280) : (GetST() ? 210 : 250)); }

    inline uint32_t GetLineDisplayTime() const
    {
        return 41000;
    }

    enum InternalRegistersMemoryMap
    {
        CSRW = 0x00,
        CSRR = 0x01,
        DCR  = 0x02,
        VSR  = 0x04,
        BCR  = 0x06, // LSB only
        DCR2 = 0x08,
        DCP  = 0x0A,
        SWM  = 0x0C, // MSB only
        STM  = 0x0E, // LSB only

        A     = 0x10,
        B     = 0x12,
        PCR   = 0x14,
        MASK  = 0x16, // Low order nibble only
        SHIFT = 0x18, // bits 8-9 only
        INDEX = 0x1A, // bits 0-1 only
        FCBC  = 0x1C, // FC and BC are respectively MSB and LSB of this register.
        TC    = 0x1E, // MSB only
    };
};

#endif // CDI_CORES_SCC66470_SCC66470_HPP
