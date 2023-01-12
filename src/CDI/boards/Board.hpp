#ifndef CDI_BOARDS_BOARD_HPP
#define CDI_BOARDS_BOARD_HPP

class CDI;
#include "../CDIConfig.hpp"
#include "../common/types.hpp"
#include "../common/Video.hpp"
#include "../cores/IRTC.hpp"
#include "../cores/ISlave.hpp"
#include "../cores/SCC68070/SCC68070.hpp"
#include "../OS9/BIOS.hpp"

#include <memory>

class Board
{
public:
    const std::string name;
    CDI& cdi;

    SCC68070 cpu;
    std::unique_ptr<ISlave> slave;
    std::unique_ptr<IRTC> timekeeper;

    Board(CDI& idc, const std::string& name, const CDIConfig& conf) : name(name), cdi(idc), cpu(idc, conf.PAL ? SCC68070::PAL_FREQUENCY : SCC68070::NTSC_FREQUENCY), slave(), timekeeper() {}
    virtual ~Board() {  }
    virtual void Reset(const bool resetCPU) = 0;
    virtual void IncrementTime(const double ns) { slave->IncrementTime(ns); timekeeper->IncrementClock(ns); }

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Trigger | Log) = 0;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Trigger | Log) = 0;
    virtual uint32_t GetLong(const uint32_t addr, const uint8_t flags = Trigger | Log) = 0;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Trigger | Log) = 0;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Trigger | Log) = 0;
    virtual void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Trigger | Log) = 0;

    virtual uint32_t GetRAMSize() const = 0;
    virtual RAMBank GetRAMBank1() const = 0;
    virtual RAMBank GetRAMBank2() const = 0;
    virtual const uint8_t* GetPointer(const uint32_t addr) const
    {
        const RAMBank ram1 = GetRAMBank1();
        const RAMBank ram2 = GetRAMBank2();
        const OS9::BIOS& bios = GetBIOS();

        if(addr >= ram1.base && addr < ram1.base + ram1.data.size())
            return &ram1.data[addr - ram1.base];

        if(addr >= ram2.base && addr < ram2.base + ram2.data.size())
            return &ram2.data[addr - ram2.base];

        if(addr >= bios.base && addr < bios.base + bios.size)
            return bios(addr - bios.base);

        return nullptr;
    }

    virtual uint32_t GetTotalFrameCount() = 0;
    virtual const OS9::BIOS& GetBIOS() const = 0;

    virtual std::vector<InternalRegister> GetInternalRegisters() = 0;
    virtual std::vector<InternalRegister> GetControlRegisters() = 0;
    virtual const Video::Plane& GetScreen() = 0;
    virtual const Video::Plane& GetPlaneA() = 0;
    virtual const Video::Plane& GetPlaneB() = 0;
    virtual const Video::Plane& GetBackground() = 0;
    virtual const Video::Plane& GetCursor() = 0;
};

#endif // CDI_BOARDS_BOARD_HPP
