#ifndef VDSC_HPP
#define VDSC_HPP

class VDSC;

#include <string>
#include <vector>
#include <functional>

#include <wx/image.h>

#include "../common/flags.hpp"
class Board;

struct VDSCRegister
{
    std::string name;
    uint32_t address;
    uint32_t value;
    std::string disassembledValue;
};

class VDSC
{
protected:
    uint16_t lineNumber; // lines starts at 0

public:
    Board* board;
    bool biosLoaded;
    uint32_t allocatedMemory;
    uint32_t totalFrameCount;
    std::vector<std::string> ICA1;
    std::vector<std::string> DCA1;
    std::vector<std::string> ICA2;
    std::vector<std::string> DCA2;

    VDSC(Board* baord) : lineNumber(0), board(baord), biosLoaded(false), allocatedMemory(0), totalFrameCount(0) {}
    virtual ~VDSC() {}

    virtual void Reset() = 0;

    virtual bool LoadBIOS(const void* bios, const uint32_t size) = 0;
    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) = 0;
    virtual void MemorySwap() = 0;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Log | Trigger) = 0;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger) = 0;
    virtual uint32_t GetLong(const uint32_t addr, const uint8_t flags = Log | Trigger) = 0;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Log | Trigger) = 0;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger) = 0;
    virtual void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Log | Trigger) = 0;

    virtual void DrawLine() = 0;
    virtual inline uint32_t GetLineDisplayTimeNanoSeconds() { return 0; }

    virtual void SetOnFrameCompletedCallback(std::function<void()> callback) = 0;
    virtual void StopOnNextFrame(const bool stop = true) = 0;

    virtual std::vector<VDSCRegister> GetInternalRegisters() = 0;
    virtual std::vector<VDSCRegister> GetControlRegisters() = 0;
    virtual wxImage GetScreen() = 0;
    virtual wxImage GetPlaneA() = 0;
    virtual wxImage GetPlaneB() = 0;
    virtual wxImage GetBackground() = 0;
    virtual wxImage GetCursor() = 0;
};

#endif // VDSC_HPP
