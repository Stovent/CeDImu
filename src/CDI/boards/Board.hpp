#ifndef BOARD_HPP
#define BOARD_HPP

class Board;

#include "../cores/SCC68070/SCC68070.hpp"
#include "../cores/MC68HC705C8/MC68HC705C8.hpp"
#include "../cores/M48T08/M48T08.hpp"
#include "../cores/VDSC.hpp"
#include "../common/flags.hpp"

class Board
{
public:
    SCC68070 cpu;
    MC68HC705C8 slave;
    M48T08 timekeeper;

    Board(const void* slaveBios, const uint16_t slaveSize, const bool initNVRAMClock) : cpu(this), slave(slaveBios, slaveSize), timekeeper(initNVRAMClock) {}
    virtual ~Board() {  }
    virtual void Reset(const bool resetCPU) = 0;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Trigger | Log) = 0;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Trigger | Log) = 0;
    virtual uint32_t GetLong(const uint32_t addr, const uint8_t flags = Trigger | Log) = 0;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Trigger | Log) = 0;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Trigger | Log) = 0;
    virtual void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Trigger | Log) = 0;

    virtual uint8_t CPUGetUART(const uint8_t flags = Trigger | Log) = 0;
    virtual void CPUSetUART(const uint8_t data, const uint8_t flags = Trigger | Log) = 0;

    virtual void DrawLine() = 0;
    virtual uint32_t GetLineDisplayTime() = 0;

    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) = 0;
    virtual void WriteToBIOSArea(const void* s, unsigned int size, unsigned int position) = 0;
    virtual void StopOnNextFrame(const bool stop = true) = 0;

    virtual uint32_t GetAllocatedMemory() = 0;
    virtual uint32_t GetTotalFrameCount() = 0;
    virtual void SetOnFrameCompletedCallback(std::function<void()> callback) = 0;

    virtual std::vector<std::string> GetICA1() = 0;
    virtual std::vector<std::string> GetDCA1() = 0;
    virtual std::vector<std::string> GetICA2() = 0;
    virtual std::vector<std::string> GetDCA2() = 0;

    virtual std::vector<VDSCRegister> GetInternalRegisters() = 0;
    virtual std::vector<VDSCRegister> GetControlRegisters() = 0;
    virtual Plane GetScreen() = 0;
    virtual Plane GetPlaneA() = 0;
    virtual Plane GetPlaneB() = 0;
    virtual Plane GetBackground() = 0;
    virtual Plane GetCursor() = 0;
};

#endif // BOARD_HPP
