#ifndef VDSC_HPP
#define VDSC_HPP

class VDSC;

#include <string>

class VDSC
{
protected:
    uint16_t lineNumber;

public:
    uint8_t* memory;
    bool biosLoaded;
    uint32_t allocatedMemory;

    VDSC() : lineNumber(0), memory(nullptr), allocatedMemory(0) {}
    virtual ~VDSC() {}

    virtual bool LoadBIOS(std::string filename) = 0;
    virtual void PutDataInMemory(const uint8_t* s, unsigned int size, unsigned int position) = 0;
    virtual void ResetMemory() = 0;
    virtual void MemorySwap();

    virtual uint8_t  GetByte(const uint32_t& addr) const = 0;
    virtual uint16_t GetWord(const uint32_t& addr) = 0;
    virtual uint32_t GetLong(const uint32_t& addr) = 0;

    virtual void SetByte(const uint32_t& addr, const uint8_t& data) = 0;
    virtual void SetWord(const uint32_t& addr, const uint16_t& data) = 0;
    virtual void SetLong(const uint32_t& addr, const uint32_t& data) = 0;

    virtual void DisplayLine() = 0;
};

#endif // VDSC_HPP
