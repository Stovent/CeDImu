#ifndef MCD212_HPP
#define MCD212_HPP

class MCD212;

#include <fstream>

#include "../VDSC.hpp"

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

enum CodingMethods
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

    virtual inline uint32_t GetLineDisplayTimeNanoSeconds() override // as nano seconds
    {
        return GetCF() ? (GetST() ? 48000 : 51200) : 51400;
    }

    virtual void SetOnFrameCompletedCallback(std::function<void()> callback) override;
    virtual void StopOnNextFrame(const bool stop = true) override;

    virtual void DrawLine() override;

    virtual std::vector<VDSCRegister> GetInternalRegisters() override;
    virtual std::vector<VDSCRegister> GetControlRegisters() override;
    virtual wxImage GetScreen() override;
    virtual wxImage GetPlaneA() override;
    virtual wxImage GetPlaneB() override;
    virtual wxImage GetBackground() override;
    virtual wxImage GetCursor() override;

private:
    uint8_t* memory;

    wxImage screen;
    wxImage planeA;
    wxImage planeB;
    wxImage cursorPlane;
    wxImage backgroundPlane;
    uint8_t memorySwapCount;

    bool stopOnNextFrame;
    uint32_t* controlRegisters;
    uint16_t* internalRegisters;
    uint32_t CLUT[256];
    const uint8_t dequantizer[16] = {0, 1, 4, 9, 16, 27, 44, 79, 128, 177, 212, 229, 240, 247, 252, 255};
    std::ofstream out_dram;
    std::ofstream out_display;
    std::function<void()> OnFrameCompleted;

    void DrawLinePlaneA();
    void DrawLinePlaneB();
    void DrawLineCursor();
    void DrawLineBackground();

    // Display File Decoders
    void DecodeBitmapLineA();
    void DecodeBitmapLineB();
    void DecodeRunLengthLine(wxImage& plane, void (MCD212::*CLUTDecoder)(const uint8_t, uint8_t[3], const uint8_t), uint8_t* data, bool cm);
    void DecodeMosaicLineA();
    void DecodeMosaicLineB();

    // Real-Time Decoders (set pixels in RGB format)
    uint8_t DecodeRGB555(const uint16_t pixel, uint8_t pixels[3]); // returns the alpha byte
    void DecodeDYUV(const uint16_t pixel, uint8_t pixels[6], const uint32_t previous);
    void DecodeCLUTA(const uint8_t pixel, uint8_t pixels[3], const uint8_t CLUTType);
    void DecodeCLUTB(const uint8_t pixel, uint8_t pixels[3], const uint8_t CLUTType);

    void ExecuteICA1();
    void ExecuteDCA1();
    void ExecuteICA2();
    void ExecuteDCA2();

    // internal registers
    uint16_t GetCSR1RRegister();
    uint16_t GetCSR1WRegister();
    uint16_t GetDCR1Register();
    uint16_t GetVSR1Register();
    uint16_t GetDDR1Register();
    uint16_t GetDCP1Register();

    uint16_t GetCSR2RRegister();
    uint16_t GetCSR2WRegister();
    uint16_t GetDCR2Register();
    uint16_t GetVSR2Register();
    uint16_t GetDDR2Register();
    uint16_t GetDCP2Register();

    std::string DisassembleCSR1RRegister();
    std::string DisassembleCSR1WRegister();
    std::string DisassembleDCR1Register();
    std::string DisassembleVSR1Register();
    std::string DisassembleDDR1Register();
    std::string DisassembleDCP1Register();

    std::string DisassembleCSR2RRegister();
    std::string DisassembleCSR2WRegister();
    std::string DisassembleDCR2Register();
    std::string DisassembleVSR2Register();
    std::string DisassembleDDR2Register();
    std::string DisassembleDCP2Register();

    bool GetDA();
    bool GetPA();
    bool GetIT1();
    bool GetIT2();
    bool GetBE_R();
    bool GetBE_W();
    bool GetDI1();
    uint8_t GetDD12();
    bool GetTD();
    bool GetDD();
    bool GetST();
    bool GetDI2();
    bool GetDE();
    bool GetCF();
    bool GetFD();
    bool GetSM();
    bool GetCM1();
    bool GetIC1();
    bool GetDC1();
    bool GetCM2();
    bool GetIC2();
    bool GetDC2();
    uint8_t GetMF12_1();
    uint8_t GetFT12_1();
    uint8_t GetMF12_2();
    uint8_t GetFT12_2();
    uint32_t GetVSR1();
    uint32_t GetVSR2();
    uint32_t GetDCP1();
    uint32_t GetDCP2();

    void SetIT1(const bool it = 1);
    void SetIT2(const bool it = 1);
    void SetDCP1(const uint32_t value);
    void SetVSR1(const uint32_t value);
    void SetDCP2(const uint32_t value);
    void SetVSR2(const uint32_t value);
    void ReloadDisplayParameters1(const bool dm, const uint8_t MF, const uint8_t FT);
    void ReloadDisplayParameters2(const bool dm, const uint8_t MF, const uint8_t FT);

    inline uint16_t GetHorizontalResolution1() { uint16_t a = GetCF() ? (GetST() ? 360 : 384) : 360; return GetCM1() ? a*2 : a; }
    inline uint16_t GetHorizontalResolution2() { uint16_t a = GetCF() ? (GetST() ? 360 : 384) : 360; return GetCM2() ? a*2 : a; }
    inline uint16_t GetVerticalResolution() { return GetFD() ? 240 : (GetST() ? 240 : 280); }
};

#endif // MCD212_HPP
