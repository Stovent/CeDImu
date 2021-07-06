#ifndef BOARD_HPP
#define BOARD_HPP

class CDI;
#include "../CDIConfig.hpp"
#include "../common/enums.hpp"
#include "../cores/IRTC.hpp"
#include "../cores/ISlave.hpp"
#include "../cores/VDSC.hpp"
#include "../cores/SCC68070/SCC68070.hpp"

#include <memory>

class Board
{
public:
    const std::string name;
    CDI& cdi;

    SCC68070 cpu;
    std::unique_ptr<ISlave> slave;
    std::unique_ptr<IRTC> timekeeper;

    Board(CDI& idc, const std::string& name, const CDIConfig& conf) : name(name), cdi(idc), cpu(idc, conf.PAL ? SCC68070_PAL_FREQUENCY : SCC68070_NTSC_FREQUENCY) {}
    virtual ~Board() {  }
    virtual void Reset(const bool resetCPU) = 0;

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

        if(addr >= ram1.base && addr < ram1.base + ram1.size)
            return &ram1.data[addr - ram1.base];

        if(addr >= ram2.base && addr < ram2.base + ram2.size)
            return &ram2.data[addr - ram2.base];

        if(addr >= bios.base && addr < bios.base + bios.size)
            return bios(addr - bios.base);

        return nullptr;
    }

    virtual void ExecuteVideoLine() = 0;
    virtual uint32_t GetLineDisplayTime() = 0;

    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) = 0;
    virtual void WriteToBIOSArea(const void* s, unsigned int size, unsigned int position) = 0;

    virtual uint32_t GetTotalFrameCount() = 0;
    virtual const OS9::BIOS& GetBIOS() const = 0;

    virtual std::vector<VDSCRegister> GetInternalRegisters() = 0;
    virtual std::vector<VDSCRegister> GetControlRegisters() = 0;
    virtual const Plane& GetScreen() = 0;
    virtual const Plane& GetPlaneA() = 0;
    virtual const Plane& GetPlaneB() = 0;
    virtual const Plane& GetBackground() = 0;
    virtual const Plane& GetCursor() = 0;
};

#endif // BOARD_HPP
