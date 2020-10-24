#ifndef SCC66470_HPP
#define SCC66470_HPP

class SCC66470;

#include "../VDSC.hpp"

#include <fstream>

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

    std::ofstream out_dram;

public:
    explicit SCC66470(Board* board, const bool ismaster);
    virtual ~SCC66470();

    virtual void Reset() override;

    virtual bool LoadBIOS(const void* bios, const uint32_t size) override;
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
    virtual inline uint32_t GetLineDisplayTimeNanoSeconds() override
    {
        return 41000;
    }

    virtual void SetOnFrameCompletedCallback(std::function<void()> callback) override;
    virtual void StopOnNextFrame(const bool stop = true) override;

    virtual std::vector<VDSCRegister> GetInternalRegisters() override;
    virtual std::vector<VDSCRegister> GetControlRegisters() override;
    virtual wxImage GetScreen() override;
    virtual wxImage GetPlaneA() override;
    virtual wxImage GetPlaneB() override;
    virtual wxImage GetBackground() override;
    virtual wxImage GetCursor() override;

    // Internal Register
    uint16_t GetCSRWRegister();
    uint16_t GetCSRRRegister();
    uint16_t GetDCRRegister();
    uint16_t GetVSRRegister();
    uint16_t GetBCRRegister();
    uint16_t GetDCR2Register();
    uint16_t GetDCPRegister();
    uint16_t GetSWMRegister();
    uint16_t GetSTMRegister();
    uint16_t GetARegister();
    uint16_t GetBRegister();
    uint16_t GetPCRRegister();
    uint16_t GetMASKRegister();
    uint16_t GetSHIFTRegister();
    uint16_t GetINDEXRegister();
    uint16_t GetFCRegister();
    uint16_t GetBCRegister();
    uint16_t GetTCRegister();
};

#endif // SCC66470_HPP
