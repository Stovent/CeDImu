#ifndef VDSC_HPP
#define VDSC_HPP

class VDSC;

#include <string>
#include <map>

#include "../CeDImu.hpp"

struct VDSCRegister
{
    uint32_t address;
    uint16_t value;
};

class VDSC
{
protected:
    uint16_t lineNumber; // lines starts at 0

public:
    CeDImu* app;
    uint8_t* memory;
    bool biosLoaded;
    bool stopOnNextCompletedFrame;
    uint32_t allocatedMemory;
    uint32_t totalFrameCount;
    std::string biosFilename;

    VDSC(CeDImu* appp) : lineNumber(0), app(appp), memory(nullptr), biosLoaded(false), stopOnNextCompletedFrame(false), allocatedMemory(0), totalFrameCount(0) {}
    virtual ~VDSC() { delete[] memory; }

    virtual void Reset() = 0;

    virtual bool LoadBIOS(const char* filename) = 0;
    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) = 0;
    virtual void MemorySwap() = 0;

    // do NOT use those for emulation
    // they are here for reading memory without logs and without affecting emulation
    virtual uint8_t  GetByteNoDebug(const uint32_t addr) = 0;
    virtual uint16_t GetWordNoDebug(const uint32_t addr) = 0;
    virtual uint32_t GetLongNoDebug(const uint32_t addr) = 0;

    virtual uint8_t  GetByte(const uint32_t addr) = 0;
    virtual uint16_t GetWord(const uint32_t addr) = 0;
    virtual uint32_t GetLong(const uint32_t addr) = 0;

    virtual void SetByte(const uint32_t addr, const uint8_t data) = 0;
    virtual void SetWord(const uint32_t addr, const uint16_t data) = 0;
    virtual void SetLong(const uint32_t addr, const uint32_t data) = 0;

    virtual void DisplayLine() = 0;
    virtual inline uint32_t GetLineDisplayTimeNanoSeconds() { return 0; }

    virtual void OnFrameCompleted() = 0;

    virtual std::map<std::string, VDSCRegister> GetInternalRegisters() = 0;
};

#endif // VDSC_HPP
