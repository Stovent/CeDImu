#ifndef MINIMMC_HPP
#define MINIMMC_HPP

class MiniMMC;

#include "Board.hpp"
#include "../cores/SCC66470/SCC66470.hpp"

class MiniMMC : public Board
{
    std::ofstream out;
    std::ofstream uart_out;
    std::ifstream uart_in;

public:
    MiniMMC(const void* bios, const uint32_t size);
    virtual ~MiniMMC();
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
};

#endif // MINIMMC_HPP
