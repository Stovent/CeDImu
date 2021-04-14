#ifndef VDSC_HPP
#define VDSC_HPP

class VDSC;

class Board;
#include "../common/flags.hpp"
#include "../OS9/BIOS.hpp"

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

struct RAMBank
{
    const uint8_t* data;
    uint32_t base;
    uint32_t size;
};

class VDSC
{
public:
    Board& board;
    OS9::BIOS BIOS;
    uint32_t totalFrameCount;
    std::vector<std::string> ICA1;
    std::vector<std::string> DCA1;
    std::vector<std::string> ICA2;
    std::vector<std::string> DCA2;

    VDSC() = delete;
    VDSC(Board& baord, const void* bios, const uint32_t size) : board(baord), BIOS(bios, size), totalFrameCount(0) {}
    virtual ~VDSC() {}

    virtual void Reset() = 0;

    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) = 0;
    virtual void WriteToBIOSArea(const void* s, unsigned int size, unsigned int position) = 0;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Log | Trigger) = 0;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger) = 0;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Log | Trigger) = 0;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger) = 0;

    virtual RAMBank GetRAMBank1() const = 0;
    virtual RAMBank GetRAMBank2() const = 0;

    virtual void ExecuteVideoLine() = 0;
    virtual inline uint32_t GetLineDisplayTime() const { return 0; }

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
