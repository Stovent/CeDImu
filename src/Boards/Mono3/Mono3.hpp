#ifndef MONO3_HPP
#define MONO3_HPP

class Mono3;

#include <fstream>

#include "../Board.hpp"
#include "../../cores/MCD212/MCD212.hpp"

class Mono3 : public Board
{
    MCD212 mcd212;
    std::ofstream out;
    std::ofstream uart_out;
    std::ifstream uart_in;

public:
    Mono3(const void* vdscBios, const uint32_t vdscSize, const void* slaveBios, const uint16_t slaveSize);
    virtual ~Mono3();
    virtual void Reset() override;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Trigger | Log) override;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Trigger | Log) override;
    virtual uint32_t GetLong(const uint32_t addr, const uint8_t flags = Trigger | Log) override;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Trigger | Log) override;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Trigger | Log) override;
    virtual void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Trigger | Log) override;

    virtual uint8_t CPUGetUART(const uint8_t flags = Trigger | Log) override;
    virtual void CPUSetUART(const uint8_t data, const uint8_t flags = Trigger | Log) override;

    virtual void DrawLine() override;
    virtual uint32_t GetLineDisplayTime() override;

    virtual void PutDataInMemory(const void* s, unsigned int size, unsigned int position) override;
    virtual void StopOnNextFrame(const bool stop = true) override;

    virtual uint32_t GetAllocatedMemory() override;
    virtual uint32_t GetTotalFrameCount() override;
    virtual void SetOnFrameCompletedCallback(std::function<void()> callback) override;

    virtual std::vector<std::string> GetICA1() override;
    virtual std::vector<std::string> GetDCA1() override;
    virtual std::vector<std::string> GetICA2() override;
    virtual std::vector<std::string> GetDCA2() override;

    virtual std::vector<VDSCRegister> GetInternalRegisters() override;
    virtual std::vector<VDSCRegister> GetControlRegisters() override;
    virtual wxImage GetScreen() override;
    virtual wxImage GetPlaneA() override;
    virtual wxImage GetPlaneB() override;
    virtual wxImage GetBackground() override;
    virtual wxImage GetCursor() override;
};

#endif // MONO3_HPP
