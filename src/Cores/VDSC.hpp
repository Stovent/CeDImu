#ifndef VDSC_HPP
#define VDSC_HPP

class VDSC;

#include "../CeDImu.hpp"

class VDSC
{
protected:
    uint16_t lineNumber;

public:
    CeDImu* app;
    uint8_t* memory;
    bool biosLoaded;
    uint32_t allocatedMemory;

    VDSC(CeDImu* appp) : lineNumber(0), memory(nullptr), allocatedMemory(0) { app = appp; }
    virtual ~VDSC() {}

    virtual void Reset() = 0;

    virtual bool LoadBIOS(const char* filename) = 0;
    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) = 0;
    virtual void ResetMemory() = 0;
    virtual void MemorySwap() = 0;

    virtual uint8_t  GetByte(const uint32_t& addr) = 0;
    virtual uint16_t GetWord(const uint32_t& addr) = 0;
    virtual uint32_t GetLong(const uint32_t& addr) = 0;

    virtual void SetByte(const uint32_t& addr, const uint8_t& data) = 0;
    virtual void SetWord(const uint32_t& addr, const uint16_t& data) = 0;
    virtual void SetLong(const uint32_t& addr, const uint32_t& data) = 0;

    virtual void DisplayLine() = 0;
    virtual uint32_t GetLineDisplayTime() = 0;
};

#endif // VDSC_HPP
