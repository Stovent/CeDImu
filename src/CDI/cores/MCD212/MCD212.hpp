#ifndef MCD212_HPP
#define MCD212_HPP

class MCD212;

#include "../VDSC.hpp"

#include <fstream>

enum MCD212Registers
{
    MCSR2W = 0x00,
    MCSR2R = 0x01,
    MDCR2  = 0x02,
    MVSR2  = 0x04,
    MDDR2  = 0x08,
    MDCP2  = 0x0A,
    MCSR1W = 0x10,
    MCSR1R = 0x11,
    MDCR1  = 0x12,
    MVSR1  = 0x14,
    MDDR1  = 0x18,
    MDCP1  = 0x1A,
};

enum MCD212ControlRegistersMap
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

enum ImageCodingMethods
{
    OFF    = 0b0000,
    CLUT8  = 0b0001,
    RGB555 = 0b0001,
    CLUT7  = 0b0011,
    CLUT77 = 0b0100,
    DYUV   = 0b0101,
    CLUT4  = 0b1011,
};

class MCD212 : public VDSC
{
public:
    explicit MCD212(Board* board);
    virtual ~MCD212();

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

    virtual inline uint32_t GetLineDisplayTimeNanoSeconds() const override // as nano seconds
    {
        return GetCF() ? (GetST() ? 48000 : 51200) : 51400;
    }

    virtual void SetOnFrameCompletedCallback(std::function<void()> callback) override;
    virtual void StopOnNextFrame(const bool stop = true) override;

    virtual void DrawLine() override;

    virtual std::vector<VDSCRegister> GetInternalRegisters() const override;
    virtual std::vector<VDSCRegister> GetControlRegisters() const override;
    virtual Plane GetScreen() const override;
    virtual Plane GetPlaneA() const override;
    virtual Plane GetPlaneB() const override;
    virtual Plane GetBackground() const override;
    virtual Plane GetCursor() const override;

private:
    uint8_t memorySwapCount;

    uint8_t* memory;

    uint8_t* screen;
    uint8_t* planeA;
    uint8_t* planeB;
    uint8_t* cursorPlane;
    uint8_t* backgroundPlane;

    bool stopOnNextFrame;
    uint32_t* controlRegisters;
    uint16_t* internalRegisters;
    uint32_t CLUT[256];
    std::ofstream out_dram;
    std::ofstream out_display;
    std::function<void()> OnFrameCompleted;

    void DrawLinePlaneA();
    void DrawLinePlaneB();
    void DrawLineCursor();
    void DrawLineBackground();

    // Display File Decoders
    void DecodeMosaicLineA();
    void DecodeMosaicLineB();

    // Control Area
    void ExecuteICA1();
    void ExecuteDCA1();
    void ExecuteICA2();
    void ExecuteDCA2();

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

    void SetIT1(const bool it = 1);
    void SetIT2(const bool it = 1);
    void SetDCP1(const uint32_t value);
    void SetVSR1(const uint32_t value);
    void SetDCP2(const uint32_t value);
    void SetVSR2(const uint32_t value);
    void ReloadDisplayParameters1(const bool dm, const uint8_t MF, const uint8_t FT);
    void ReloadDisplayParameters2(const bool dm, const uint8_t MF, const uint8_t FT);

    inline uint16_t GetHorizontalResolution1() const { uint16_t a = GetCF() ? (GetST() ? 360 : 384) : 360; return GetCM1() ? a*2 : a; }
    inline uint16_t GetHorizontalResolution2() const { uint16_t a = GetCF() ? (GetST() ? 360 : 384) : 360; return GetCM2() ? a*2 : a; }
    inline uint16_t GetVerticalResolution() const { return GetFD() ? 240 : (GetST() ? 240 : 280); }
};

#endif // MCD212_HPP
