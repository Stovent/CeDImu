#ifndef VDSC_HPP
#define VDSC_HPP

class VDSC;

#include <string>

class VDSC
{
protected:
    int8_t* memory;
    uint16_t lineNumber;

public:
    VDSC() : memory(nullptr), lineNumber(0) {}
    virtual ~VDSC() {}

    virtual void LoadBIOS(std::string filename) = 0;

    virtual int8_t  GetByte(const uint32_t addr) const = 0;
    virtual int16_t GetWord(const uint32_t addr) const = 0;
    virtual int32_t GetLong(const uint32_t addr) const = 0;

    virtual void SetByte(const uint32_t addr, int8_t data) = 0;
    virtual void SetWord(const uint32_t addr, int16_t data) = 0;
    virtual void SetLong(const uint32_t addr, int32_t data) = 0;

    virtual void DisplayLine() = 0;
};

#endif // VDSC_HPP
