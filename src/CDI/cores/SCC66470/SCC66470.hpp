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
    SBCR  = 0x07,
    SDCR2 = 0x08,
    SDCP  = 0x0A,
    SSWM  = 0x0C,
    SSTM  = 0x0F,

    SA     = 0x10,
    SB     = 0x12,
    SPCR   = 0x14,
    SMASK  = 0x17,
    SSHIFT = 0x18,
    SINDEX = 0x1B,
    SFC    = 0x1C,
    SBC    = 0x1D,
    STC    = 0x1E,
};

class SCC66470 : public VDSC
{
    uint8_t* memory;
    uint16_t* internalRegisters;
    uint8_t memorySwapCount;
    bool stopOnNextFrame;
    const bool isMaster;
    std::function<void()> OnFrameCompleted;

    FILE* out_dram;

public:
    explicit SCC66470(Board* board, const bool ismaster);
    virtual ~SCC66470();

    virtual void Reset() override;

    virtual bool LoadBIOS(const void* bios, uint32_t size) override;
    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) override;
    virtual void WriteToBIOSArea(const void* s, unsigned int size, unsigned int position) override;
    virtual void MemorySwap() override;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Log | Trigger) override;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger) override;
    virtual uint32_t GetLong(const uint32_t addr, const uint8_t flags = Log | Trigger) override;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Log | Trigger) override;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger) override;
    virtual void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Log | Trigger) override;

    virtual void DrawLine() override;
    virtual inline uint32_t GetLineDisplayTimeNanoSeconds() const override
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
};

#endif // SCC66470_HPP
