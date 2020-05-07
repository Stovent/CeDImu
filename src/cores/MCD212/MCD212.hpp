#ifndef MCD212_HPP
#define MCD212_HPP

class MCD212;

#include <fstream>

#include <wx/image.h>

#include "../VDSC.hpp"

enum MCD212Registers
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

enum CLUTTypes
{
    CLUT8  = 0b0001,
    CLUT7  = 0b0011,
    CLUT77 = 0b0100,
    CLUT4  = 0b1011,
};

class MCD212 : public VDSC
{
    wxImage planeA;
    wxImage planeB;
    wxImage cursorPlane;
    wxImage backgroundPlane;
    uint8_t memorySwapCount;
    bool isCA;

    uint32_t* controlRegisters;
    uint16_t* internalRegisters;
    uint32_t CLUT[256];
    std::ofstream out;

    void DrawLineA();
    void DrawLineB();
    void DrawCursor();
    void DrawBackground();

    // Display File Decoders
    void DecodeBitmap(wxImage& plane, uint8_t* data, bool cm);
    void DecodeRunLength(wxImage& plane, uint8_t* data, bool cm);
    void DecodeMosaic(wxImage& plane, uint8_t* data, bool cm);

    // Real-Time Decoders (set pixels in RGB format)
    uint8_t DecodeRGB555(const uint16_t pixel, uint8_t pixels[3]); // returns the alpha byte
    void DecodeDYUV(uint16_t pixel, uint32_t startValue, uint8_t pixels[6]);
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

    bool GetDA();
    bool GetPA();
    uint8_t GetIT12();
    bool GetBE();
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


public:
    MCD212(CeDImu* appp);
    virtual ~MCD212();

    virtual void Reset() override;

    virtual bool LoadBIOS(const char* filename) override;
    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) override;
    virtual void MemorySwap() override;

    virtual uint8_t  GetByteNoDebug(const uint32_t addr) override;
    virtual uint16_t GetWordNoDebug(const uint32_t addr) override;
    virtual uint32_t GetLongNoDebug(const uint32_t addr) override;

    virtual uint8_t  GetByte(const uint32_t addr) override;
    virtual uint16_t GetWord(const uint32_t addr) override;
    virtual uint32_t GetLong(const uint32_t addr) override;

    virtual void SetByte(const uint32_t addr, const uint8_t data) override;
    virtual void SetWord(const uint32_t addr, const uint16_t data) override;
    virtual void SetLong(const uint32_t addr, const uint32_t data) override;

    virtual inline uint32_t GetLineDisplayTimeNanoSeconds() override // as nano seconds
    {
        return GetCF() ? (GetST() ? 48000 : 51200) : 51400;
    }

    virtual void OnFrameCompleted() override;

    virtual void DrawLine() override;

    virtual std::map<std::string, VDSCRegister> GetInternalRegisters() override;
};

#endif // MCD212_HPP
