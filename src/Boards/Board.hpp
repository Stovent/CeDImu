#ifndef BOARD_HPP
#define BOARD_HPP

class Board;

#include "../cores/SCC68070/SCC68070.hpp"
#include "../common/flags.hpp"

class Board
{
public:
    SCC68070* cpu;

    Board() { cpu = new SCC68070(this); }
    virtual ~Board() { delete cpu; }
    virtual void Reset() = 0;

    virtual uint8_t  GetByte(const uint32_t addr, const uint8_t flags = Trigger | Log) = 0;
    virtual uint16_t GetWord(const uint32_t addr, const uint8_t flags = Trigger | Log) = 0;
    virtual uint32_t GetLong(const uint32_t addr, const uint8_t flags = Trigger | Log) = 0;

    virtual void SetByte(const uint32_t addr, const uint8_t  data, const uint8_t flags = Trigger | Log) = 0;
    virtual void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Trigger | Log) = 0;
    virtual void SetLong(const uint32_t addr, const uint32_t data, const uint8_t flags = Trigger | Log) = 0;

    virtual uint8_t GetUART(const uint8_t flags = Trigger | Log) = 0;
    virtual void SetUART(const uint8_t data, const uint8_t flags = Trigger | Log) = 0;

    virtual void DrawLine() = 0;
    virtual uint32_t GetLineDisplayTime() = 0;
};

#endif // BOARD_HPP
