#include "IKAT.hpp"

namespace HLE
{
// https://github.com/cdifan/cdichips/blob/master/mc6805ikat.md
static uint8_t responseCF4[3] = {0xA5, 0xF4, 0}; // Boot mode: 1 for service shell, 0 for player shell
static uint8_t responseCF6[4] = {0xA5, 0xF6, 1, 0xFF}; // Video standard: 1 for NTSC, 2 for PAL
static uint8_t responseDB0[3] = {0xB0, 0x02, 0x10}; // Disc status: 0x0210 according to cdiemu
static uint8_t responseDB1[4] = {0xB1, 0, 2, 0}; // Disc base: 00:02:00
static uint8_t responseDB2[4] = {0xB2, 0x20, 0, 0x10}; // Disc select: 0x200010 according to cdiemu

IKAT::IKAT(SCC68070& cpu, const bool PAL) : ISlave(cpu)
{
    index = -1;
    commandSize = 0;
    responseCF6[2] = PAL + 1;
}

uint8_t IKAT::GetByte(const uint8_t addr)
{
    if(index >= 0 && index < commandSize && (addr == PCRD || addr == PDRD))
        return command[index++];

    return 0;
}

void IKAT::SetByte(const uint8_t addr, const uint8_t data)
{
    index = 0;
    switch(addr)
    {
    case PCWR:
        switch(data)
        {
        case 0xF4:
            command = responseCF4;
            commandSize = 3;
            break;

        case 0xF6:
            command = responseCF6;
            commandSize = 4;
            break;

        default:
            index = -1;
        }
        break;

    case PDWR:
        switch(data)
        {
        case 0xB0:
            command = responseDB0;
            commandSize = 3;
            break;

        case 0xB1:
            command = responseDB1;
            commandSize = 4;
            break;

        case 0xB2:
            command = responseDB2;
            commandSize = 4;
            break;

        default:
            index = -1;
        }
        break;

    default:
        index = -1;
    }
}

} // namespace HLE
