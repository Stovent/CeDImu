#ifndef MONO3_HPP
#define MONO3_HPP

class Mono3;

#include <fstream>

#include "Board.hpp"
#include "../cores/MCD212/MCD212.hpp"

class Mono3 : Board
{
    MCD212* mcd212;
    std::ofstream out;
    std::ofstream uart_out;
    std::ifstream uart_in;

public:
    Mono3(const void* bios, const uint32_t size);
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
};

#endif // MONO3_HPP
