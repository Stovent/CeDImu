#ifndef SCC66470_HPP
#define SCC66470_HPP

class SCC66470;

#include "../VDSC.hpp"

#include <cstdio>

enum SCC66470Registers
{
    SCSRW = 0x00,
    SCSRR = 0x01,
    SDCR  = 0x02,
    SVSR  = 0x04,
    SBCR  = 0x06, // LSB only
    SDCR2 = 0x08,
    SDCP  = 0x0A,
    SSWM  = 0x0C, // MSB only
    SSTM  = 0x0E, // LSB only

    SA     = 0x10,
    SB     = 0x12,
    SPCR   = 0x14,
    SMASK  = 0x16, // Low order nibble only
    SSHIFT = 0x18, // bits 8-9 only
    SINDEX = 0x1A, // bits 0-1 only
    SFCBC  = 0x1C, // FC and BC are respectively MSB and LSB of this register.
    STC    = 0x1E, // MSB only
};

class SCC66470 : public VDSC
{
    std::vector<uint8_t> memory;
    uint8_t memorySwapCount;
    uint16_t lineNumber; // starts at 0

    std::array<uint16_t, 0x20> internalRegisters;
    uint8_t registerCSR;
    uint16_t registerB;

    bool stopOnNextFrame;
    const bool isMaster;
    std::function<void()> OnFrameCompleted;

    FILE* out_dram;

    void MemorySwap();
    uint32_t GetLong(const uint32_t addr, const uint8_t flags = Trigger);

public:
    explicit SCC66470(Board& board, const bool ismaster, const void* bios, const uint32_t size);
    virtual ~SCC66470();

    virtual void Reset() override;

    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) override;
    virtual void WriteToBIOSArea(const void* s, unsigned int size, unsigned int position) override;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Log | Trigger) override;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger) override;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Log | Trigger) override;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger) override;

    virtual RAMBank GetRAMBank1() const override;
    virtual RAMBank GetRAMBank2() const override;

    virtual void ExecuteVideoLine() override;
    virtual inline uint32_t GetLineDisplayTime() const override
    {
        return 41000;
    }

    virtual void SetOnFrameCompletedCallback(std::function<void()> callback) override;
    virtual void StopOnNextFrame(const bool stop = true) override;

    virtual std::vector<VDSCRegister> GetInternalRegisters() const override;
    virtual std::vector<VDSCRegister> GetControlRegisters() const override;
    virtual Plane GetScreen() const override;
    virtual Plane GetPlaneA() const override;
    virtual Plane GetPlaneB() const override;
    virtual Plane GetBackground() const override;
    virtual Plane GetCursor() const override;

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
};

#endif // SCC66470_HPP
