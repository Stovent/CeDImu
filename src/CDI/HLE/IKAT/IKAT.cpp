#include "IKAT.hpp"

namespace HLE
{

static uint8_t responseF4[3] = {0xA5, 0xF4, 0}; // 1 for service shell, 0 for player shell
static uint8_t responseF6[4] = {0xA5, 0xF6, 1, 0xFF}; // 1 for NTSC, 2 for PAL

IKAT::IKAT()
{
    index = -1;
}

uint8_t IKAT::GetByte(const uint8_t addr)
{
    if(addr == PCRD && index >= 0 && index < commandSize)
        return command[index++];

    return 0;
}

void IKAT::SetByte(const uint8_t addr, const uint8_t data)
{
    index = 0;
    if(addr == PCWR)
    {
        switch(data)
        {
        case 0xF4:
            command = responseF4;
            commandSize = 3;
            break;

        case 0xF6:
            command = responseF6;
            commandSize = 4;
            break;

        default:
            index = -1;
        }
    }
    else
        index = -1;
}

} // namespace HLE
