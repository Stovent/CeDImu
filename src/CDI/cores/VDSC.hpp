#ifndef VDSC_HPP
#define VDSC_HPP

class VDSC;

class Board;
#include "../common/flags.hpp"

#include <string>
#include <vector>
#include <functional>

struct VDSCRegister
{
    std::string name;
    uint32_t address;
    uint32_t value;
    std::string disassembledValue;
};

struct Plane
{
    uint8_t* pixels;
    uint16_t width;
    uint16_t height;
};

class VDSC
{
protected:
    uint16_t lineNumber; // lines starts at 0

public:
    Board& board;
    bool biosLoaded;
    uint32_t allocatedMemory;
    uint32_t totalFrameCount;
    std::vector<std::string> ICA1;
    std::vector<std::string> DCA1;
    std::vector<std::string> ICA2;
    std::vector<std::string> DCA2;

    VDSC(Board& baord) : lineNumber(0), board(baord), biosLoaded(false), allocatedMemory(0), totalFrameCount(0) {}
    virtual ~VDSC() {}

    virtual void Reset() = 0;

    virtual bool LoadBIOS(const void* bios, uint32_t size) = 0;
    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) = 0;
    virtual void WriteToBIOSArea(const void* s, unsigned int size, unsigned int position) = 0;
    virtual void MemorySwap() = 0;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Log | Trigger) = 0;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger) = 0;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Log | Trigger) = 0;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger) = 0;

    virtual void DrawLine() = 0;
    virtual inline uint32_t GetLineDisplayTimeNanoSeconds() const { return 0; }

    virtual void SetOnFrameCompletedCallback(std::function<void()> callback) = 0;
    virtual void StopOnNextFrame(const bool stop = true) = 0;

    virtual std::vector<VDSCRegister> GetInternalRegisters() const = 0;
    virtual std::vector<VDSCRegister> GetControlRegisters() const = 0;
    virtual Plane GetScreen() const = 0;
    virtual Plane GetPlaneA() const = 0;
    virtual Plane GetPlaneB() const = 0;
    virtual Plane GetBackground() const = 0;
    virtual Plane GetCursor() const = 0;
};

#endif // VDSC_HPP
