#ifndef MCD212_HPP
#define MCD212_HPP

class MCD212;

#include <fstream>

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

class MCD212 : public VDSC
{
    uint8_t memorySwapCount;

    // registers and bits
    uint16_t GetCSR1R();
    uint16_t GetCSR1W();
    uint16_t GetDCR1();
    uint16_t GetVSR1();
    uint16_t GetDDR1();
    uint16_t GetDCP1();

    uint16_t GetCSR2R();
    uint16_t GetCSR2W();
    uint16_t GetDCR2();
    uint16_t GetVSR2();
    uint16_t GetDDR2();
    uint16_t GetDCP2();

public:
    uint16_t* registers;
    std::ofstream out;

    MCD212(CeDImu* appp);
    virtual ~MCD212();

    virtual bool LoadBIOS(const char* filename) override;
    virtual void PutDataInMemory(const uint8_t* s, unsigned int size, unsigned int position) override;
    virtual void ResetMemory() override;
    virtual void MemorySwap() override;

    virtual uint8_t  GetByte(const uint32_t& addr) override;
    virtual uint16_t GetWord(const uint32_t& addr) override;
    virtual uint32_t GetLong(const uint32_t& addr) override;

    virtual void SetByte(const uint32_t& addr, const uint8_t& data) override;
    virtual void SetWord(const uint32_t& addr, const uint16_t& data) override;
    virtual void SetLong(const uint32_t& addr, const uint32_t& data) override;

    virtual void DisplayLine() override;
    virtual uint32_t GetLineDisplayTime() override;
};

#endif // MCD212_HPP
