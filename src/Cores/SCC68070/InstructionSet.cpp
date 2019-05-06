#include <wx/msgdlg.h>

#include "SCC68070.hpp"

#include "../../utils.hpp"

void SCC68070::Exception(const uint8_t& vectorNumber, uint16_t& calcTime)
{
    uint16_t sr = SR;
    SetS();

    if(vectorNumber == 2 || vectorNumber == 3) // TODO: implement long Stack format
    {
        int32_t last = lastAddress;
        SetWord(ARIWPr(7, 2), 0); // internal information
        SetWord(ARIWPr(7, 2), currentOpcode); // IRC
        SetWord(ARIWPr(7, 2), currentOpcode); // IR
        SetLong(ARIWPr(7, 4), 0); // DBIN
        SetLong(ARIWPr(7, 4), last); // TPF
        SetLong(ARIWPr(7, 4), 0); // TPD
        SetWord(ARIWPr(7, 2), 0); // internal information
        SetWord(ARIWPr(7, 2), 0); // internal information
        SetWord(ARIWPr(7, 2), 0); // MM
        SetWord(ARIWPr(7, 2), 0); // SSW
        SetWord(ARIWPr(7, 2), 0xF000 | ((uint16_t)vectorNumber << 2));
    }
    else
        SetWord(ARIWPr(7, 2), (uint16_t)vectorNumber << 2);

    SetLong(ARIWPr(7, 4), PC);
    SetWord(ARIWPr(7, 2), sr);

    if(vectorNumber <= 1 || vectorNumber == 9 || vectorNumber == 24)
        stop = false;
    switch(vectorNumber) // handle Exception Processing Clock Periods
    {
    case 2: case 3:
        calcTime += 158; break;
    case 4: case 8: case 9:
        calcTime += 55; break;
    case 32: case 33: case 34: case 35: case 36: case 37: case 38: case 39:
    case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47:
        calcTime += 52; break;
    case 5:
        calcTime += 64; break;
    default:
        if(vectorNumber == 15 || (vectorNumber >= 24 && vectorNumber < 32) || vectorNumber >= 57)
            calcTime += 65;
    break;
    }

    PC = GetLong(vectorNumber * 4);
    SetS(0);
}

uint16_t SCC68070::UnknownInstruction()
{
    instructionsBuffer.push_back(std::to_string(PC-2) + "\tUnknown instruction");
    return 0;
}

uint16_t SCC68070::Abcd()
{
    uint32_t pc = PC-2;
    uint8_t Rx = (currentOpcode & 0x0E00) >> 9;
    uint8_t Ry = (currentOpcode & 0x0007);
    uint8_t result;
    uint16_t calcTime;
    uint8_t x = GetX();

    if(currentOpcode & 0x0008) // R/M = 1 : Memory to Memory
    {
        uint8_t src = GetByte(ARIWPr(Ry, 1));
        uint8_t dst = GetByte(ARIWPr(Rx, 1));
        if((src & 0x0F) + (dst & 0x0F) > 9)
            SetXC();
        if(((src & 0xF0) >> 4) + ((dst & 0xF0) >> 4) > 9)
            SetXC();

        result = convertPBCD(src) + convertPBCD(dst) + x;
        SetByte(lastAddress, result);
        calcTime = 31;
    }
    else // R/M = 0 : Data register to Data register
    {
        uint8_t src = D[Ry] & 0x000000FF;
        uint8_t dst = D[Rx] & 0x000000FF;
        if((src & 0x0F) + (dst & 0x0F) > 9)
            SetXC();
        if(((src & 0xF0) >> 4) + ((dst & 0xF0) >> 4) > 9)
            SetXC();

        result = convertPBCD(src) + convertPBCD(dst) + x;
        D[Rx] &= 0xFFFFFF00;
        D[Rx] |= result;
        calcTime = 10;
    }

    if(result != 0)
        SetZ(0);

    instructionsBuffer.push_back(std::to_string(pc) + "\tABCD");

    return calcTime;
}

uint16_t SCC68070::Add()
{
    uint32_t pc = PC-2;
    uint8_t REGISTER = (currentOpcode & 0x0E00) >> 9;
    uint8_t   OPMODE = (currentOpcode & 0x01C0) >> 6;
    uint8_t   EAMODE = (currentOpcode & 0x0038) >> 3;
    uint8_t    EAREG = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(OPMODE == 0)
    {
        int8_t src = GetByte(EAMODE, EAREG, calcTime);
        int8_t dst = D[REGISTER] & 0x000000FF;
        int16_t res = src + dst;
        uint16_t ures = (uint8_t)src + (uint8_t)dst;

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80) SetN();
        else SetN(0);

        D[REGISTER] &= 0xFFFFFF00;
        D[REGISTER] |= (res & 0xFF);

    }
    else if(OPMODE == 1)
    {
        int16_t src = GetWord(EAMODE, EAREG, calcTime);
        int16_t dst = D[REGISTER] & 0x0000FFFF;
        int32_t res = src + dst;
        uint32_t ures = (uint16_t)src + (uint16_t)dst;

        if(ures & 0x10000) SetXC();
        else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x8000) SetN();
        else SetN(0);

        D[REGISTER] &= 0xFFFF0000;
        D[REGISTER] |= (res & 0xFFFF);
    }
    else if(OPMODE == 2)
    {
        int32_t src = GetLong(EAMODE, EAREG, calcTime);
        int32_t dst = D[REGISTER];
        int64_t res = src + dst;
        uint64_t ures = (uint32_t)src + (uint32_t)dst;

        if(ures & 0x100000000) SetXC();
        else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80000000) SetN();
        else SetN(0);

        D[REGISTER] = res;
    }
    else if(OPMODE == 4)
    {
        int8_t src = D[REGISTER] & 0x000000FF;
        int8_t dst = GetByte(EAMODE, EAREG, calcTime);
        int16_t res = src + dst;
        uint16_t ures = (uint8_t)src + (uint8_t)dst;

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80) SetN();
        else SetN(0);

        SetByte(lastAddress, res);

        calcTime += 4;
    }
    else if(OPMODE == 5)
    {
        int16_t src = D[REGISTER] & 0x0000FFFF;
        int16_t dst = GetWord(EAMODE, EAREG, calcTime);
        int32_t res = src + dst;
        uint32_t ures = (uint16_t)src + (uint16_t)dst;

        if(ures & 0x10000) SetXC();
        else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x8000) SetN();
        else SetN(0);

        SetWord(lastAddress, res);

        calcTime += 4;
    }
    else
    {
        int32_t src = D[REGISTER];
        int32_t dst = GetLong(EAMODE, EAREG, calcTime);
        int64_t res = src + dst;
        uint64_t ures = (uint32_t)src + (uint32_t)dst;

        if(ures & 0x100000000) SetXC();
        else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80000000) SetN();
        else SetN(0);

        SetLong(lastAddress, res);

        calcTime += 8;
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tADD");

    return calcTime;
}

uint16_t SCC68070::Adda()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t   size = (currentOpcode & 0x0100) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size) // Long
    {
        int32_t src = GetLong(eamode, eareg, calcTime);
        A[reg] += src;
    }
    else // Word
    {
        int16_t src = GetWord(eamode, eareg, calcTime);
        int16_t dst = A[reg] & 0x0000FFFF;
        A[reg] = src + dst;
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tADDA");

    return calcTime;
}

uint16_t SCC68070::Addi()
{
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0b0000000011000000) >> 6;
    uint8_t eamode = (currentOpcode & 0b0000000000111000) >> 3;
    uint8_t  eareg = (currentOpcode & 0b0000000000000111);
    uint16_t calcTime = 14;

    if(size == 0) // Byte
    {
        int8_t src = GetNextWord() & 0x00FF;
        int8_t dst = GetByte(eamode, eareg, calcTime);
        int16_t res = src + dst;
        uint16_t ures = (uint8_t)src + (uint8_t)dst;

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80) SetN();
        else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= (res & 0xFF); }
        else
        {   SetByte(lastAddress, res); calcTime += 4; }
    }
    else if(size == 1) // Word
    {
        int16_t src = GetNextWord();
        int16_t dst = GetWord(eamode, eareg, calcTime);
        int32_t res = src + dst;
        uint32_t ures = (uint16_t)src + (uint16_t)dst;

        if(ures & 0x10000) SetXC();
        else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x8000) SetN();
        else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= (res & 0xFFFF); }
        else
        {   SetWord(lastAddress, res); calcTime += 4; }
    }
    else if(size == 2) // Long
    {
        int32_t src = (GetNextWord() << 16) | GetNextWord();
        int32_t dst = GetLong(eamode, eareg, calcTime);
        int64_t res = src + dst;
        uint64_t ures = (uint32_t)src + (uint32_t)dst;

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(eamode == 0)
        {   D[eareg] = res; calcTime += 4; }
        else
        {   SetLong(lastAddress, res); calcTime += 12; }
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tADDI");

    return calcTime;
}

uint16_t SCC68070::Addq()
{
    uint32_t pc = PC-2;
    uint8_t   doto = (currentOpcode & 0x0E00) >> 9;
    uint8_t   data = doto ? doto : 8;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(eamode == 1)
    {
        if(size == 1)
        {
            int16_t dst = A[eareg] & 0x0000FFFF;
            A[eareg] = data + dst;
        }
        else
            A[eareg] += data;
    }
    else if(size == 0)
    {
        int8_t dst = GetByte(eamode, eareg, calcTime);
        int16_t res = data + dst;
        uint16_t ures = data + (uint8_t)dst;

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80) SetN();
        else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= (res & 0xFF); }
        else
        {   SetByte(lastAddress, res); calcTime += 4; }
    }
    else if(size == 1)
    {
        int16_t dst = GetWord(eamode, eareg, calcTime);
        int32_t res = data + dst;
        uint32_t ures = data + (uint16_t)dst;

        if(ures & 0x10000) SetXC();
        else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x8000) SetN();
        else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= (res & 0xFFFF); }
        else
        {   SetByte(lastAddress, res); calcTime += 4; }
    }
    else
    {
        int32_t dst = GetLong(eamode, eareg, calcTime);
        int64_t res = data + dst;
        uint64_t ures = data + (uint32_t)dst;

        if(ures & 0x100000000) SetXC();
        else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(eamode == 0)
            D[eareg] = res;
        else
        {   SetByte(lastAddress, res); calcTime += 8; }
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tADDQ");

    return calcTime;
}

uint16_t SCC68070::Addx()
{
    uint32_t pc = PC-2;
    uint8_t   Rx = (currentOpcode & 0x0E00) >> 9;
    uint8_t   Ry = (currentOpcode & 0x0007);
    uint8_t size = (currentOpcode & 0x00C0) >> 6;
    uint8_t   rm = (currentOpcode & 0x0008) >> 3;
    uint16_t calcTime = 7;

    if(size == 0)
    {
        int8_t src; int8_t dst;
        if(rm)
        {   src = GetByte(ARIWPr(Ry, 1));
            dst = GetByte(ARIWPr(Rx, 1)); calcTime += 2 * ITARIWPrBW + 21; }
        else
        {   src = D[Ry] & 0x000000FF;
            dst = D[Rx] & 0x000000FF; }

        int16_t res = src + dst + GetX();
        uint16_t ures = (uint8_t)src + (uint8_t)dst + GetX();

        if(ures & 0x100) SetXC();
        else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80) SetN();
        else SetN(0);

        if(rm)
            SetByte(lastAddress, res);
        else
        {   D[Rx] &= 0xFFFFFF00; D[Rx] |= (res & 0xFF); }
    }
    else if(size == 1)
    {
        int16_t src; int16_t dst;
        if(rm)
        {   src = GetWord(ARIWPr(Ry, 2));
            dst = GetWord(ARIWPr(Rx, 2)); calcTime += 2 * ITARIWPrBW + 21; }
        else
        {   src = D[Ry] & 0x0000FFFF;
            dst = D[Rx] & 0x0000FFFF; }

        int32_t res = src + dst + GetX();
        uint32_t ures = (uint16_t)src + (uint16_t)dst + GetX();

        if(ures & 0x10000) SetXC();
        else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x8000) SetN();
        else SetN(0);

        if(rm)
            SetWord(lastAddress, res);
        else
        {   D[Rx] &= 0xFFFF0000; D[Rx] |= (res & 0xFFFF); }
    }
    else
    {
        int32_t src; int32_t dst;
        if(rm)
        {   src = GetLong(ARIWPr(Ry, 4));
            dst = GetLong(ARIWPr(Rx, 4)); calcTime += 2 * ITARIWPrL + 33; }
        else
        {   src = D[Ry];
            dst = D[Rx]; }

        int64_t res = src + dst + GetX();
        uint64_t ures = (uint32_t)src + (uint32_t)dst + GetX();

        if(ures & 0x100000000) SetXC();
        else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV();
        else SetV(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(rm)
            SetLong(lastAddress, res);
        else
            D[Rx] = res;
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tADDX");

    return calcTime;
}

uint16_t SCC68070::And()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(opmode == 0)
    {
        uint8_t src = GetByte(eamode, eareg, calcTime);
        uint8_t dst = D[reg] & 0x000000FF;
        uint8_t res = src & dst;

        if(res & 0x80) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        D[reg] &= 0xFFFFFF00;
        D[reg] |= res;
    }
    else if(opmode == 1)
    {
        uint16_t src = GetWord(eamode, eareg, calcTime);
        uint16_t dst = D[reg] & 0x0000FFFF;
        uint16_t res = src & dst;

        if(res & 0x8000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        D[reg] &= 0xFFFF0000;
        D[reg] |= res;
    }
    else if(opmode == 2)
    {
        uint32_t src = GetLong(eamode, eareg, calcTime);
        uint32_t dst = D[reg];
        uint32_t res = src & dst;

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        D[reg] = res;
    }
    else if(opmode == 3)
    {
        uint8_t src = D[reg] & 0x000000FF;
        uint8_t dst = GetByte(eamode, eareg, calcTime);
        uint8_t res = src & dst;

        if(res & 0x80) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        SetByte(lastAddress, res);

        calcTime += 4;
    }
    else if(opmode == 4)
    {
        uint16_t src = D[reg] & 0x0000FFFF;
        uint16_t dst = GetWord(eamode, eareg, calcTime);
        uint16_t res = src & dst;

        if(res & 0x8000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        SetWord(lastAddress, res);

        calcTime += 4;
    }
    else
    {
        uint32_t src = D[reg];
        uint32_t dst = GetLong(eamode, eareg, calcTime);
        uint32_t res = src & dst;

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        SetLong(lastAddress, res);

        calcTime += 8;
    }

    SetC(0);
    SetV(0);
    instructionsBuffer.push_back(std::to_string(pc) + "\tAND");
    return calcTime;
}

uint16_t SCC68070::Andi()
{
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;

    if(size == 0)
    {
        uint8_t data = GetNextWord() & 0xFF;
        uint8_t  dst = GetByte(eamode, eareg, calcTime);
        uint8_t  res = data & dst;

        if(res & 0x80) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(eamode)
        {   SetByte(lastAddress, res); calcTime += 4; }
        else
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= res; }
    }
    else if(size == 1)
    {
        uint16_t data = GetNextWord();
        uint16_t  dst = GetWord(eamode, eareg, calcTime);
        uint16_t  res = data & dst;

        if(res & 0x8000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(eamode)
        {   SetWord(lastAddress, res); calcTime += 4; }
        else
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= res; }
    }
    else
    {
        uint32_t data = (GetNextWord() << 16) | GetNextWord();
        uint32_t  dst = GetLong(eamode, eareg, calcTime);
        uint32_t  res = data & dst;

        if(res & 0x80000000) SetN();
        else SetN(0);

        if(res == 0) SetZ();
        else SetZ(0);

        if(eamode)
        {   SetWord(lastAddress, res); calcTime += 10; }
        else
        {   D[eareg] = res; calcTime += 4; }
    }

    SetV(0);
    SetC(0);
    instructionsBuffer.push_back(std::to_string(pc) + "\tANDI");
    return calcTime;
}

uint16_t SCC68070::AsM()
{
    uint32_t pc = PC-2;
    uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;
    uint16_t data = GetWord(eamode, eareg, calcTime);

    bool b = false;
    if(dr) // left
    {
        uint8_t a = data & 0x8000;
        SetXC(a);
        data <<= 1;
        if(a != (data & 0x8000))
            b = true;
        instructionsBuffer.push_back(std::to_string(pc) + "\tASL");
    }
    else // right
    {
        uint16_t msb = data & 0x8000;
        uint16_t a = data & 0x0001;
        SetXC(a);
        data >>= 1;
        data |= msb;
        instructionsBuffer.push_back(std::to_string(pc) + "\tASR");
    }

    if(data & 0x8000) SetN(); else SetN(0);
    if(data == 0) SetZ(); else SetZ(0);
    if(b) SetV(); else SetV(0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::AsR()
{
    uint32_t pc = PC-2;
    uint8_t count = (currentOpcode & 0x0E00) >> 9;
    uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    uint8_t   reg = (currentOpcode & 0x0007);
    uint8_t shift;
    if(ir)
        shift = D[reg] % 64;
    else
        shift = (count) ? count : 8;

    if(!shift)
    {
        SetV(0);
        SetC(0);
        return 13;
    }

    bool b = false;
    if(size == 0) // byte
    {
        uint8_t data = D[reg] & 0x000000FF;
        if(dr)
        {
            uint8_t old = data & 0x80;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x80;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            uint8_t msb = data & 0x80;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x01;
                SetXC(a);
                data >>= 1;
                data |= msb;
            }
        }

        if(data & 0x80) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] &= 0xFFFFFF00;
        D[reg] |= data;
    }
    else if(size == 1) // word
    {
        uint16_t data = D[reg] & 0x0000FFFF;
        if(dr)
        {
            uint16_t old = data & 0x8000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint16_t a = data & 0x8000;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            uint16_t msb = data & 0x8000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint16_t a = data & 0x0001;
                SetXC(a);
                data >>= 1;
                data |= msb;
            }
        }

        if(data & 0x8000) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] &= 0xFFFF0000;
        D[reg] |= data;
    }
    else // long
    {
        uint32_t data = D[reg];
        if(dr)
        {
            uint32_t old = data & 0x80000000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint32_t a = data & 0x80000000;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            uint32_t msb = data & 0x80000000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint32_t a = data & 0x00000001;
                SetXC(a);
                data >>= 1;
                data |= msb;
            }
        }

        if(data & 0x80000000) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] = data;
    }

    if(b)
        SetV();
    else
        SetV(0);

    if(dr)
        instructionsBuffer.push_back(std::to_string(pc) + "\tASL");
    else
        instructionsBuffer.push_back(std::to_string(pc) + "\tASR");

    return 13 + 3 * shift;
}

uint16_t SCC68070::BCC()
{
    uint32_t pc = PC-2;
    uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    int8_t       disp = (currentOpcode & 0x00FF);
    uint16_t calcTime;

    if(disp) // 8-bit
    {
        calcTime = 13;
        if((this->*ConditionalTests[condition])())
            PC += disp;
    }
    else // 16 bit
    {
        calcTime = 14;
        if((this->*ConditionalTests[condition])())
            PC += (int16_t)GetNextWord() - 2;
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tBCC");

    return calcTime;
}

uint16_t SCC68070::Bchg()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;
    uint8_t shift;
    if(currentOpcode & 0x0100)
    {   shift = D[reg] % 32; calcTime = 10; }
    else
    {   shift = (GetNextWord() & 0x00FF) % 8; calcTime = 17; }

    if(eamode == 0)
    {
        uint32_t data = D[eareg];
        uint32_t mask = 1 << shift;
        if(data & mask)
        {
            SetZ(0);
            data &= ~(mask);
        }
        else
        {
            SetZ();
            data |= mask;
        }
        D[eareg] = data;
    }
    else
    {
        calcTime += 4;
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t mask = 1 << shift;
        if(data & mask)
        {
            SetZ(0);
            data &= ~(mask);
        }
        else
        {
            SetZ();
            data |= mask;
        }
        SetByte(lastAddress, data);
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tBCHG");

    return calcTime;
}

uint16_t SCC68070::Bclr()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;
    uint8_t shift;
    if(currentOpcode & 0x0100)
    {   shift = D[reg] % 32; calcTime = 10; }
    else
    {   shift = (GetNextWord() & 0x00FF) % 8; calcTime = 17; }

    if(eamode == 0)
    {
        uint32_t data = D[eareg];
        uint32_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        data &= ~(mask);
        D[eareg] = data;
    }
    else
    {
        calcTime += 4;
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        data &= ~(mask);
        SetByte(lastAddress, data);
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tBCLR");

    return calcTime;
}

uint16_t SCC68070::Bra()
{
    uint32_t pc = PC-2;
    int8_t disp = currentOpcode & 0x00FF;
    uint16_t calcTime;

    if(disp)
    {
        PC += disp;
        calcTime = 13;
    }
    else
    {
        PC += (int16_t)GetNextWord() - 2;
        calcTime = 14;
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tBRA");

    return calcTime;
}

uint16_t SCC68070::Bset()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;
    uint8_t shift;
    if(currentOpcode & 0x0100)
    {   shift = D[reg] % 32; calcTime = 10; }
    else
    {   shift = (GetNextWord() & 0x00FF) % 8; calcTime = 17; }

    if(eamode == 0)
    {
        uint32_t data = D[eareg];
        uint32_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        data |= mask;
        D[eareg] = data;
    }
    else
    {
        calcTime += 4;
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        data |= mask;
        SetByte(lastAddress, data);
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tBSET");

    return calcTime;
}

uint16_t SCC68070::Bsr()
{
    uint32_t pc = PC-2;
    int8_t disp = currentOpcode & 0x00FF;
    uint16_t calcTime;

    if(disp)
    {
        SetLong(ARIWPr(7, 4), PC);
        PC += disp;
        calcTime = 21;
    }
    else
    {
        int16_t dsp = GetNextWord();
        SetLong(ARIWPr(7, 4), PC);
        PC += dsp - 2;
        calcTime = 25;
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tBSR");

    return calcTime;
}

uint16_t SCC68070::Btst()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;
    uint8_t shift;
    if(currentOpcode & 0x0100)
    {   shift = D[reg] % 32; calcTime = 7; }
    else
    {   shift = (GetNextWord() & 0x00FF) % 8; calcTime = 14; }

    if(eamode == 0)
    {
        uint32_t data = D[eareg];
        uint32_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        D[eareg] = data;
    }
    else
    {
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t mask = 1 << shift;
        if(data & mask)
            SetZ(0);
        else
            SetZ();
        SetByte(lastAddress, data);
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tBTST");

    return calcTime;
}

uint16_t SCC68070::Chk()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 19;

    int16_t source = GetWord(eamode, eareg, calcTime);
    int16_t data = D[reg] & 0xFFFF;

    if(data < 0 || data > source)
    {
        calcTime += 45;
        Exception(6, calcTime);
        if(data < 0) SetN();
        if(data > source) SetN(0);
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tCHK");

    return calcTime;
}

uint16_t SCC68070::Clr()
{
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 0) // Byte
        SetByte(eamode, eareg, calcTime, 0);
    else if(size == 1) // Word
        SetWord(eamode, eareg, calcTime, 0);
    else // Long
        SetLong(eamode, eareg, calcTime, 0);

    SetN(0);
    SetZ();
    SetVC(0);
    instructionsBuffer.push_back(std::to_string(pc) + "\tCLR");
    return calcTime;
}

uint16_t SCC68070::Cmp()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(opmode == 0) // Byte
    {
        int8_t dst = D[reg] &= 0x000000FF;
        int8_t src = GetByte(eamode, eareg, calcTime);
        int16_t res = dst - src;
        uint16_t ures = (uint8_t)dst - (uint8_t)src;

        if(ures & 0x100) SetC(); else SetC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);
    }
    else if(opmode == 1) // Word
    {
        int16_t dst = D[reg] &= 0x0000FFFF;
        int16_t src = GetWord(eamode, eareg, calcTime);
        int32_t res = dst - src;
        uint32_t ures = (uint16_t)dst - (uint16_t)src;

        if(ures & 0x10000) SetC(); else SetC(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);
    }
    else // Long
    {
        int32_t dst = D[reg];
        int32_t src = GetLong(eamode, eareg, calcTime);
        int64_t res = dst - src;
        uint64_t ures = (uint32_t)dst - (uint32_t)src;

        if(ures & 0x100000000) SetC(); else SetC(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);
    }
    instructionsBuffer.push_back(std::to_string(pc) + "\tCMP");
    return calcTime;
}

uint16_t SCC68070::Cmpa()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x0100) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(opmode) // Long
    {
        int32_t dst = A[reg];
        int32_t src = GetLong(eamode, eareg, calcTime);
        int64_t res = dst - src;
        uint64_t ures = (uint32_t)dst - (uint32_t)src;

        if(ures & 0x100000000) SetC(); else SetC(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);
    }
    else // Word
    {
        int16_t dst = A[reg] &= 0x0000FFFF;
        int16_t src = signExtend16(GetWord(eamode, eareg, calcTime));
        int32_t res = dst - src;
        uint32_t ures = (uint16_t)dst - (uint16_t)src;

        if(ures & 0x10000) SetC(); else SetC(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);
    }
    instructionsBuffer.push_back(std::to_string(pc) + "\tCMPA");
    return calcTime;
}

uint16_t SCC68070::Cmpi()
{
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;

    if(size == 0) // Byte
    {
        int8_t data = GetNextWord() & 0x00FF;
        int8_t dst = GetByte(eamode, eareg, calcTime);
        int16_t res = dst - data;
        uint16_t ures = (uint8_t)dst - (uint8_t)data;

        if(ures & 0x100) SetC(); else SetC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);
    }
    else if(size == 1) // Word
    {
        int16_t data = GetNextWord();
        int16_t dst = GetWord(eamode, eareg, calcTime);
        int32_t res = dst - data;
        uint32_t ures = (uint16_t)dst - (uint16_t)data;

        if(ures & 0x10000) SetC(); else SetC(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);
    }
    else // Long
    {
        int32_t data = (GetNextWord() << 16) | GetNextWord();
        int32_t dst = GetByte(eamode, eareg, calcTime);
        int64_t res = dst - data;
        uint64_t ures = (uint32_t)dst - (uint32_t)data;

        if(ures & 0x100000000) SetC(); else SetC(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tCMPI");
    return (size == 2) ? calcTime + 4 : calcTime;
}

uint16_t SCC68070::Cmpm()
{
    uint32_t pc = PC-2;
    uint8_t size = (currentOpcode & 0x00C0) >> 6;
    uint8_t Ax = (currentOpcode & 0x0E00) >> 9;
    uint8_t Ay = (currentOpcode & 0x0007);

    if(size == 0) // byte
    {
        int8_t src = GetByte(ARIWPo(Ay, 1));
        int8_t dst = GetByte(ARIWPo(Ax, 1));
        int16_t res = dst - src;

        if(res == 0) SetZ(); else SetZ(0);
        if(res < 0) SetN(); else SetZ(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if((dst < src) ^ (((dst ^ src) >= 0) == false)) SetC(); else SetC(0); // From bizhawk : C = ((a < b) ^ ((a ^ b) >= 0) == false);
    }
    else if(size == 1) // word
    {
        int16_t src = GetWord(ARIWPo(Ay, 2));
        int16_t dst = GetWord(ARIWPo(Ax, 2));
        int32_t res = dst - src;

        if(res == 0) SetZ(); else SetZ(0);
        if(res < 0) SetN(); else SetZ(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if((dst < src) ^ (((dst ^ src) >= 0) == false)) SetC(); else SetC(0);
    }
    else // long
    {
        int32_t src = GetLong(ARIWPo(Ay, 4));
        int32_t dst = GetLong(ARIWPo(Ax, 4));
        int64_t res = dst - src;

        if(res == 0) SetZ(); else SetZ(0);
        if(res < 0) SetN(); else SetZ(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if((dst < src) ^ (((dst ^ src) >= 0) == false)) SetC(); else SetC(0);
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tCMPM");

    return (size == 2) ? 26 : 18;
}

uint16_t SCC68070::DbCC()
{
    uint32_t pc = PC-2;
    uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    uint8_t reg = (currentOpcode & 0x0007);
    int16_t disp = GetNextWord();

    instructionsBuffer.push_back(std::to_string(pc) + "\tDBcc D" + std::to_string(reg) + ", " + DisassembleConditionalCode(condition));

    if((this->*ConditionalTests[condition])())
       return 14;

    int16_t data = D[reg] & 0x0000FFFF;
    if(--data == -1)
        return 17;

    PC += signExtend16(disp) - 2;

    return 17;
}

uint16_t SCC68070::Divs()
{
    uint32_t pc = PC-2;


    return 0;
}

uint16_t SCC68070::Divu()
{
    uint32_t pc = PC-2;


    return 0;
}

uint16_t SCC68070::Eor()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x001C) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 0;

    if(opmode == 4)
    {
        uint8_t src = D[reg] & 0x000000FF;
        uint8_t dst = GetByte(eamode, eareg, calcTime);
        uint8_t res = src ^ dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        calcTime += 11;
        SetByte(lastAddress, res);
    }
    else if(opmode == 5)
    {
        uint16_t src = D[reg] & 0x0000FFFF;
        uint16_t dst = GetWord(eamode, eareg, calcTime);
        uint16_t res = src ^ dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        calcTime += 11;
        SetWord(lastAddress, res);
    }
    else
    {
        uint32_t src = D[reg];
        uint32_t dst = GetLong(eamode, eareg, calcTime);
        uint32_t res = src ^ dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        calcTime += 15;
        SetLong(lastAddress, res);
    }
    SetVC(0);

    instructionsBuffer.push_back(std::to_string(pc) + "\tEOR");
    return calcTime;
}

uint16_t SCC68070::Eori()
{
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 0;

    if(size == 0) // Byte
    {
        uint8_t data = GetNextWord() & 0x00FF;
        uint8_t dst = GetByte(eamode, eareg, calcTime);
        uint8_t res = data ^ dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        if(eamode)
        {   SetByte(lastAddress, res); calcTime += 18; }
        else
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= res; calcTime = 14; }
    }
    else if(size == 1) // Word
    {
        uint16_t data = GetNextWord();
        uint16_t dst = GetWord(eamode, eareg, calcTime);
        uint16_t res = data ^ dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        if(eamode)
        {   SetWord(lastAddress, res); calcTime += 18; }
        else
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= res; calcTime = 14; }
    }
    else // Long
    {
        uint32_t data = (GetNextWord() << 16) | GetNextWord();
        uint32_t dst = GetLong(eamode, eareg, calcTime);
        uint32_t res = data ^ dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        if(eamode)
        {   SetLong(lastAddress, res); calcTime += 26; }
        else
        {   D[eareg] = res; calcTime = 18; }
    }
    SetVC(0);

    instructionsBuffer.push_back(std::to_string(pc) + "\tEORI");
    return calcTime;
}

uint16_t SCC68070::Exg()
{
    uint32_t pc = PC-2;
    uint8_t   Rx = (currentOpcode & 0x0E00) >> 9;
    uint8_t mode = (currentOpcode & 0x00F8) >> 3;
    uint8_t   Ry = (currentOpcode & 0x0007);

    uint32_t tmp;

    if(mode == 0x08)
    {
        tmp = D[Rx];
        D[Rx] = D[Ry];
        D[Ry] = tmp;
    }
    else if(mode == 0x09)
    {
        tmp = A[Rx];
        A[Rx] = A[Ry];
        A[Ry] = tmp;
    }
    else
    {
        tmp = D[Rx];
        D[Rx] = A[Ry];
        A[Ry] = tmp;
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tEXG");

    return 13;
}

uint16_t SCC68070::Ext()
{
    uint32_t pc = PC-2;
    uint8_t mode = (currentOpcode & 0x01C0) >> 6;
    uint8_t  reg = (currentOpcode & 0x0007);

    if(mode == 2) // byte to word
    {
        uint16_t tmp = signExtend816(D[reg] & 0x000000FF);
        D[reg] &= 0xFFFF0000;
        D[reg] |= tmp;
    }
    else if(mode == 3) // word to long
    {
        D[reg] = signExtend16(D[reg] & 0x0000FFFF);
    }
    else // byte to long
    {
        D[reg] = signExtend8(D[reg] & 0x000000FF);
    }

    SetVC(0);
    instructionsBuffer.push_back(std::to_string(pc) + "\tEXT");
    return 7;
}

uint16_t SCC68070::Jmp()
{
    uint32_t pc = PC-2;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;

    if(eamode == 2)
    {   PC = A[eareg]; calcTime = 7; }
    else if(eamode == 5)
    {   PC = ARIWD(eareg); calcTime = 14; }
    else if(eamode == 6)
    {   PC = ARIWI8(eareg); calcTime = 17; }
    else
    {
        if(eareg == 0)
        {   PC = ASA(); calcTime = 14; }
        else if(eareg == 1)
        {   PC = ALA(); calcTime = 18; }
        else if(eareg == 2)
        {   PC = PCIWD(); calcTime = 14; }
        else
        {   PC = PCIWI8(); calcTime = 17; }
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tJMP " + std::to_string(PC));

    return calcTime;
}

uint16_t SCC68070::Jsr()
{
    uint32_t _pc = PC-2;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint32_t pc;
    uint16_t calcTime;

    if(eamode == 2)
    {   pc = A[eareg]; calcTime = 18; }
    else if(eamode == 5)
    {   pc = ARIWD(eareg); calcTime = 25; }
    else if(eamode == 6)
    {   pc = ARIWI8(eareg); calcTime = 28; }
    else
    {
        if(eareg == 0)
        {   pc = ASA(); calcTime = 25; }
        else if(eareg == 1)
        {   pc = ALA(); calcTime = 29; }
        else if(eareg == 2)
        {   pc = PCIWD(); calcTime = 25; }
        else
        {   pc = PCIWI8(); calcTime = 28; }
    }

    SetLong(ARIWPr(7, 4), _pc+2);

    instructionsBuffer.push_back(std::to_string(_pc) + "\tJSR " + std::to_string(pc));

    PC = pc;
    return calcTime;
}

uint16_t SCC68070::Lea()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 0;

    GetLong(eamode, eareg, calcTime);
    A[reg] = lastAddress;
    if(eamode == 7 && eareg <= 1)
        calcTime += 2;
    instructionsBuffer.push_back(std::to_string(pc) + "\tLEA");
    return calcTime;
}

uint16_t SCC68070::Link()
{
    uint32_t pc = PC-2;
    uint8_t reg = (currentOpcode & 0x0007);
    SetLong(ARIWPr(7, 4), A[reg]);
    A[reg] = SP;
    SP += signExtend16(GetNextWord());
    instructionsBuffer.push_back(std::to_string(pc) + "\tLINK");
    return 25;
}

uint16_t SCC68070::LsM()
{
    uint32_t pc = PC-2;
    uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;
    uint16_t data = GetWord(eamode, eareg, calcTime);

    bool b = false;
    if(dr) // left
    {
        uint8_t a = data & 0x8000;
        SetXC(a);
        data <<= 1;
        if(a != (data & 0x8000))
            b = true;
        instructionsBuffer.push_back(std::to_string(pc) + "\tLSL");
    }
    else // right
    {
        uint8_t a = data & 0x0001;
        SetXC(a);
        data >>= 1;
        instructionsBuffer.push_back(std::to_string(pc) + "\tLSR");
    }

    if(data & 0x8000) SetN(); else SetN(0);
    if(data == 0) SetZ(); else SetZ(0);
    if(b) SetV(); else SetV(0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::LsR()
{
    uint32_t pc = PC-2;
    uint8_t count = (currentOpcode & 0x0E00) >> 9;
    uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    uint8_t   reg = (currentOpcode & 0x0007);
    uint8_t shift;
    if(ir)
        shift = D[reg] % 64;
    else
        shift = (count) ? count : 8;

    if(dr)
        instructionsBuffer.push_back(std::to_string(pc) + "\tLSL");
    else
        instructionsBuffer.push_back(std::to_string(pc) + "\tLSR");

    if(!shift)
    {
        SetV(0);
        SetC(0);
        return 13;
    }

    bool b = false;
    if(size == 0) // byte
    {
        uint8_t data = D[reg] & 0x000000FF;
        if(dr)
        {
            uint8_t old = data & 0x80;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x80;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x01;
                SetXC(a);
                data >>= 1;
            }
        }

        if(data & 0x80) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] &= 0xFFFFFF00;
        D[reg] |= data;
    }
    else if(size == 1) // word
    {
        uint16_t data = D[reg] & 0x0000FFFF;
        if(dr)
        {
            uint8_t old = data & 0x8000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x8000;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x0001;
                SetXC(a);
                data >>= 1;
            }
        }

        if(data & 0x8000) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] &= 0xFFFF0000;
        D[reg] |= data;
    }
    else // long
    {
        uint32_t data = D[reg];
        if(dr)
        {
            uint8_t old = data & 0x80000000;
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x80000000;
                SetXC(a ? 1 : 0);
                data <<= 1;
                if(a != old)
                    b = true;
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x00000001;
                SetXC(a);
                data >>= 1;
            }
        }

        if(data & 0x80000000) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] = data;
    }

    if(b)
        SetV();
    else
        SetV(0);

    return 13 + 3 * shift;
}

uint16_t SCC68070::Move()
{
    uint32_t pc = PC-2;
    uint8_t    size = (currentOpcode & 0x3000) >> 12;
    uint8_t  dstreg = (currentOpcode & 0x0E00) >> 9;
    uint8_t dstmode = (currentOpcode & 0x01C0) >> 6;
    uint8_t srcmode = (currentOpcode & 0x0038) >> 3;
    uint8_t  srcreg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 1) // byte
    {
        uint8_t src = GetByte(srcmode, srcreg, calcTime);
        if(src == 0) SetZ(); else SetZ(0);
        if(src & 0x80) SetN(); else SetN(0);
        SetByte(dstmode, dstreg, calcTime, src);
    }
    else if(size == 3) // word
    {
        uint16_t src = GetWord(srcmode, srcreg, calcTime);
        if(src == 0) SetZ(); else SetZ(0);
        if(src & 0x8000) SetN(); else SetN(0);
        SetWord(dstmode, dstreg, calcTime, src);
    }
    else // long
    {
        uint32_t src = GetLong(srcmode, srcreg, calcTime);
        if(src == 0) SetZ(); else SetZ(0);
        if(src & 0x80000000) SetN(); else SetN(0);
        SetLong(dstmode, dstreg, calcTime, src);
    }

    SetVC(0);

    instructionsBuffer.push_back(std::to_string(pc) + "\tMOVE");

    return calcTime;
}

uint16_t SCC68070::Moveccr()
{
    uint32_t pc = PC-2;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 10;

    uint16_t data = GetWord(eamode, eareg, calcTime) & 0x00FF;
    SR &= 0xFF00;
    SR |= data;

    instructionsBuffer.push_back(std::to_string(pc) + "\tMOVECCR");

    return calcTime;
}

uint16_t SCC68070::Movesr()
{
    uint32_t pc = PC-2;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 10;

    if(GetS())
    {
        uint16_t data = GetWord(eamode, eareg, calcTime);
        SR = data;
        if(data == 0) SetZ(); else SetZ(0);
        if(data & 0x8000) SetN(); else SetN(0);
        SetV(0);
        SetC(0);
    }
    else
        Exception(8, calcTime);

    instructionsBuffer.push_back(std::to_string(pc) + "\tMOVESR");

    return calcTime;
}

uint16_t SCC68070::MoveFsr() // Should not be used according to the Green Book Chapter VI.2.2.2
{
    uint32_t pc = PC-2;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(GetS())
    {
        SetWord(eamode, eareg, calcTime, SR);
        if(eamode > 0)
            calcTime += 4;
    }
    else
        Exception(8, calcTime);

    instructionsBuffer.push_back(std::to_string(pc) + "\tMOVEfSR");

    return calcTime;
}

uint16_t SCC68070::Moveusp()
{
    uint32_t pc = PC-2;
    uint8_t  dr = (currentOpcode & 0x0008) >> 3;
    uint8_t reg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(GetS())
        if(dr)
            A[reg] = USP;
        else
            USP = A[reg];
    else
        Exception(8, calcTime);

    instructionsBuffer.push_back(std::to_string(pc) + "\tMOVEUSP");

    return calcTime;
}

uint16_t SCC68070::Movea()
{
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0x3000) >> 12;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 3) // word
    {
        int16_t data = GetWord(eamode, eareg, calcTime);
        A[reg] = signExtend16(data);
    }
    else // long
    {
        A[reg] = GetWord(eamode, eareg, calcTime);
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tMOVEA");

    return calcTime;
}

uint16_t SCC68070::Movem()
{
    uint32_t pc = PC-2;
    uint8_t     dr = (currentOpcode & 0x0400) >> 10;
    uint8_t   size = (currentOpcode & 0x0040) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t mask = GetNextWord();
    uint16_t calcTime = 0;
    bool type = false;

    GetByte(eamode, eareg, calcTime);

    uint8_t n = 0;
    int8_t mod;
    if(dr) // Memory to register
    {
        mod = 0;
        for(uint8_t i = 0; i < 16; i++)
        {
            if(mask & 1)
            {
                if(size) // long
                {
                    if(type) // address
                        A[mod] = GetLong(lastAddress);
                    else // data
                        D[mod] = GetLong(lastAddress);
                }
                else // word
                {
                    if(type) // address
                        A[mod] = signExtend16(GetWord(lastAddress));
                    else // data
                    {
                        D[mod] &= 0xFFFF0000;
                        D[mod] |= (uint16_t)GetWord(lastAddress);
                    }
                }
                n++;
                lastAddress += (size) ? 4 : 2;
            }
            mask >>= 1;
            mod++;
            mod %= 8;
            if(mod == 0)
                type = true; // false means data; true means address
        }
        if(eamode < 4 || (eamode == 7 && eareg < 2))
            calcTime += 22;
        else
            calcTime += 19;
    }
    else // Register to memory
    {
        mod = 7;
        if(eamode == 4)
            lastAddress += (size) ? 4 : 2;
        for(uint8_t i = 0; i < 16; i++)
        {
            if(mask & 1)
            {
                lastAddress -= (size) ? 4 : 2;
                if(size) // long
                {
                    if(type) // address
                        SetLong(lastAddress, D[mod]);
                    else // data
                        SetLong(lastAddress, A[mod]);
                }
                else // word
                {
                    if(type) // address
                        SetWord(lastAddress, D[mod]);
                    else // data
                        SetWord(lastAddress, A[mod]);
                }
                n++;
            }
            mask >>= 1;
            mod--;
            if(mod < 0)
            {
                type = true; // true means data; false means address
                mod = 7;
            }
        }
        if(eamode > 3 && eamode < 7)
            calcTime += 16;
        else
            calcTime += 19;
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tMOVEM");
    return calcTime + n * (size) ? 11 : 7;
}

uint16_t SCC68070::Movep()
{
    uint32_t pc = PC-2;
    wxMessageBox("Warning: MOVEP not supported yet");

    instructionsBuffer.push_back(std::to_string(pc) + "\tMOVEP");

    return 0;
}

uint16_t SCC68070::Moveq()
{
    uint32_t pc = PC-2;
    uint8_t  reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t data = (currentOpcode & 0x00FF);
    if(data & 0x80) SetN(); else SetN(0);
    if(data == 0) SetZ(); else SetZ(0);
    SetVC(0);
    D[reg] = signExtend8(data);
    instructionsBuffer.push_back(std::to_string(pc) + "\tMOVEQ");
    return 7;
}

uint16_t SCC68070::Muls()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 76;

    int16_t src = GetWord(eamode, eareg, calcTime);
    int16_t dst = D[reg] & 0x0000FFFF;

    int32_t res = src * dst;
    if(res == 0) SetZ(); else SetZ(0);
    if(res & 0x80000000) SetN(); else SetN(0);
    SetVC(0);

    D[reg] = res;
    instructionsBuffer.push_back(std::to_string(pc) + "\tMULS");
    return calcTime;
}

uint16_t SCC68070::Mulu()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 76;

    uint16_t src = GetWord(eamode, eareg, calcTime);
    uint16_t dst = D[reg] & 0x0000FFFF;

    uint32_t res = src * dst;
    if(res == 0) SetZ(); else SetZ(0);
    if(res & 0x80000000) SetN(); else SetN(0);
    SetVC(0);

    D[reg] = res;
    instructionsBuffer.push_back(std::to_string(pc) + "\tMULU");
    return calcTime;
}

uint16_t SCC68070::Nbcd()
{
    uint32_t pc = PC-2;
    uint8_t mode = (currentOpcode & 0x0038) >> 3;
    uint8_t  reg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;
    uint8_t x = GetX();
    uint8_t data = GetByte(mode, reg, calcTime);
    uint8_t result = 0 - convertPBCD(data) - x;

    if(result >= 100) SetXC(); else SetXC(0);

    if(result != 0)
        SetZ(0);

    instructionsBuffer.push_back(std::to_string(pc) + "\tNBCD");
    if(mode == 0)
    {
        D[reg] &= 0xFFFFFF00;
        D[reg] |= result;
        return 10;
    }
    SetByte(lastAddress, result);
    return calcTime - 4;
}

uint16_t SCC68070::Neg()
{
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 0) // byte
    {
        int8_t data = GetByte(eamode, eareg, calcTime);

        int16_t res = 0 - data;
        uint16_t ures = (uint8_t)0 - (uint8_t)data;

        if(ures & 0x100) SetXC(); else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV(); else SetV(0);

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        if(eamode)
        {
            SetByte(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFFFF00;
            D[eareg] |= res & 0xFF;
        }
    }
    else if(size == 1) // word
    {
        int16_t data = GetWord(eamode, eareg, calcTime);

        int32_t res = 0 - data;
        uint32_t ures = (uint16_t)0 - (uint16_t)data;

        if(ures & 0x10000) SetXC(); else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV(); else SetV(0);

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        if(eamode)
        {
            SetWord(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFF0000;
            D[eareg] |= res & 0xFFFF;
        }
    }
    else // long
    {
        int32_t data = GetLong(eamode, eareg, calcTime);

        int64_t res = 0 - data;
        uint64_t ures = (uint32_t)0 - (uint32_t)data;

        if(ures & 0x100000000) SetXC(); else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV(); else SetV(0);

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        if(eamode)
        {
            SetLong(lastAddress, res);
            calcTime += 8;
        }
        else
        {
            D[eareg] = res;
        }
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tNEG");

    return calcTime;
}

uint16_t SCC68070::Negx()
{
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 0) // byte
    {
        int8_t data = GetByte(eamode, eareg, calcTime);

        int16_t res = 0 - data - GetX();
        uint16_t ures = (uint8_t)0 - (uint8_t)data - (uint8_t)GetX();

        if(ures & 0x100) SetXC(); else SetXC(0);

        if(res > INT8_MAX || res < INT8_MIN) SetV(); else SetV(0);

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        if(eamode)
        {
            SetByte(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFFFF00;
            D[eareg] |= res & 0xFF;
        }
    }
    else if(size == 1) // word
    {
        int16_t data = GetWord(eamode, eareg, calcTime);

        int32_t res = 0 - data - GetX();
        uint32_t ures = (uint16_t)0 - (uint16_t)data - (uint16_t)GetX();

        if(ures & 0x10000) SetXC(); else SetXC(0);

        if(res > INT16_MAX || res < INT16_MIN) SetV(); else SetV(0);

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        if(eamode)
        {
            SetWord(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFF0000;
            D[eareg] |= res & 0xFFFF;
        }
    }
    else // long
    {
        int32_t data = GetLong(eamode, eareg, calcTime);

        int64_t res = 0 - data - GetX();
        uint64_t ures = (uint32_t)0 - (uint32_t)data - (uint32_t)GetX();

        if(ures & 0x100000000) SetXC(); else SetXC(0);

        if(res > INT32_MAX || res < INT32_MIN) SetV(); else SetV(0);

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        if(eamode)
        {
            SetLong(lastAddress, res);
            calcTime += 8;
        }
        else
        {
            D[eareg] = res;
        }
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tNEGX");

    return calcTime;
}

uint16_t SCC68070::Nop()
{
    instructionsBuffer.push_back(std::to_string(PC-2) + "\tNOP");
    return 7; // I love this instruction :D
}

uint16_t SCC68070::Not()
{
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7; // arbitrary, set as default

    if(size == 0) // Byte
    {
        int8_t data = GetByte(eamode,  eareg, calcTime);
        int8_t res = ~data;
        SetByte(lastAddress, res);
        calcTime += 4;

        if(res == 0)
            SetZ();
        else
            SetZ(0);
        if(res & 0x80)
            SetN();
        else
            SetN(0);
    }
    else if(size == 1) // Word
    {
        int16_t data = GetWord(eamode,  eareg, calcTime);
        int16_t res = ~data;
        SetWord(lastAddress, res);
        calcTime += 4;

        if(res == 0)
            SetZ();
        else
            SetZ(0);
        if(res & 0x8000)
            SetN();
        else
            SetN(0);
    }
    else // long
    {
        int32_t data = GetLong(eamode,  eareg, calcTime);
        int32_t res = ~data;
        SetLong(lastAddress, res);
        calcTime += 8;

        if(res == 0)
            SetZ();
        else
            SetZ(0);
        if(res & 0x80000000)
            SetN();
        else
            SetN(0);
    }
    SetVC(0);

    instructionsBuffer.push_back(std::to_string(pc) + "\tNOT");

    return calcTime;
}

uint16_t SCC68070::Or()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x001C) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 0;

    if(opmode == 0)
    {
        uint8_t src = GetByte(eamode, eareg, calcTime);
        uint8_t dst = D[reg] & 0x000000FF;
        uint8_t res = src | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] &= 0xFFFFFF00;
        D[reg] |= res;
    }
    else if(opmode == 1)
    {
        uint16_t src = GetWord(eamode, eareg, calcTime);
        uint16_t dst = D[reg] & 0x0000FFFF;
        uint16_t res = src | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] &= 0xFFFF0000;
        D[reg] |= res;
    }
    else if(opmode == 2)
    {
        uint32_t src = GetLong(eamode, eareg, calcTime);
        uint32_t dst = D[reg];
        uint32_t res = src | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] = res;
    }
    else if(opmode == 4)
    {
        uint8_t src = D[reg] & 0x000000FF;
        uint8_t dst = GetByte(eamode, eareg, calcTime);
        uint8_t res = src | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        calcTime += 11;
        SetByte(lastAddress, res);
    }
    else if(opmode == 5)
    {
        uint16_t src = D[reg] & 0x0000FFFF;
        uint16_t dst = GetWord(eamode, eareg, calcTime);
        uint16_t res = src | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        calcTime += 11;
        SetWord(lastAddress, res);
    }
    else
    {
        uint32_t src = D[reg];
        uint32_t dst = GetLong(eamode, eareg, calcTime);
        uint32_t res = src | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        calcTime += 15;
        SetLong(lastAddress, res);
    }
    SetVC(0);

    instructionsBuffer.push_back(std::to_string(pc) + "\tOR");
    return calcTime;
}

uint16_t SCC68070::Ori()
{
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 0;

    if(size == 0) // Byte
    {
        uint8_t data = GetNextWord() & 0x00FF;
        uint8_t dst = GetByte(eamode, eareg, calcTime);
        uint8_t res = data | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        if(eamode)
        {   SetByte(lastAddress, res); calcTime += 18; }
        else
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= res; calcTime = 14; }
    }
    else if(size == 1) // Word
    {
        uint16_t data = GetNextWord();
        uint16_t dst = GetWord(eamode, eareg, calcTime);
        uint16_t res = data | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        if(eamode)
        {   SetWord(lastAddress, res); calcTime += 18; }
        else
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= res; calcTime = 14; }
    }
    else // Long
    {
        uint32_t data = (GetNextWord() << 16) | GetNextWord();
        uint32_t dst = GetLong(eamode, eareg, calcTime);
        uint32_t res = data | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        if(eamode)
        {   SetLong(lastAddress, res); calcTime += 26; }
        else
        {   D[eareg] = res; calcTime = 18; }
    }
    SetVC(0);

    instructionsBuffer.push_back(std::to_string(pc) + "\tORI");
    return calcTime;
}

uint16_t SCC68070::Pea()
{
    uint32_t pc = PC-2;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 11;
    GetLong(eamode, eareg, calcTime);
    SetLong(ARIWPr(7, 4), lastAddress);
    if(eamode == 7 && eareg <= 1)
        calcTime += 2;
    instructionsBuffer.push_back(std::to_string(pc) + "\tPEA");
    return calcTime;
}

uint16_t SCC68070::Reset() // Not fully emulated I think
{
    uint16_t calcTime = 154;
    instructionsBuffer.push_back(std::to_string(PC-2) + "\tRESET");
    if(!GetS())
        Exception(8, calcTime);

    return calcTime;
}

uint16_t SCC68070::RoM()
{


    return 0;
}

uint16_t SCC68070::RoR()
{


    return 0;
}

uint16_t SCC68070::RoxM()
{
#ifdef LOG_OPCODE
    std::cout << "ROXm";
#endif // LOG_OPCODE

    return 0;
}

uint16_t SCC68070::RoxR()
{
#ifdef LOG_OPCODE
    std::cout << "ROXr";
#endif // LOG_OPCODE

    return 0;
}

uint16_t SCC68070::Rte()
{
    uint32_t pc = PC-2;
    uint16_t calcTime = 39;
    if(GetS())
    {
        SR = GetWord(ARIWPo(7, 2));
        PC = GetLong(ARIWPo(7, 4));
        if(GetWord(ARIWPo(7, 4)) & 0xF000) // long format
        {
            SP += 26;
            calcTime = 146;
        }
    }
    else
        Exception(8, calcTime);

    instructionsBuffer.push_back(std::to_string(pc) + "\tRTE");

    return calcTime;
}

uint16_t SCC68070::Rtr()
{
    uint32_t pc = PC-2;
    SR &= 0xFFE0;
    SR |= GetWord(ARIWPo(7, 2)) & 0x001F;
    PC = GetLong(ARIWPo(7, 4));
    instructionsBuffer.push_back(std::to_string(pc) + "\tRTR");
    return 22;
}

uint16_t SCC68070::Rts()
{
    uint32_t pc = PC-2;
    PC = GetLong(ARIWPo(7, 4));
    instructionsBuffer.push_back(std::to_string(pc) + "\tRTS");
    return 15;
}

uint16_t SCC68070::Sbcd()
{
    uint32_t pc = PC-2;
    uint8_t Ry = (currentOpcode & 0x0E00) >> 9;
    uint8_t Rx = (currentOpcode & 0x0007);
    uint8_t result = 0;
    uint16_t calcTime;
    uint8_t x = GetX();

    if(currentOpcode & 0x0008) // memory to memory
    {
        uint8_t src = GetByte(ARIWPr(Rx, 1));
        uint8_t dst = GetByte(ARIWPr(Ry, 1));
        result = convertPBCD(dst) - convertPBCD(src) - x;
        if(result >= 100) SetXC(); else SetXC(0);
        SetByte(lastAddress, result);
        calcTime = 31;
    }
    else
    {
        uint8_t src = D[Rx] & 0x000000FF;
        uint8_t dst = D[Ry] & 0x000000FF;
        result = convertPBCD(dst) - convertPBCD(src) - x;
        if(result >= 100) SetXC(); else SetXC(0);
        D[Ry] &= 0xFFFFFF00;
        D[Ry] |= result;
        calcTime = 10;
    }

    if(result != 0)
        SetZ(0);

    instructionsBuffer.push_back(std::to_string(pc) + "\tSBCD");

    return calcTime;
}

uint16_t SCC68070::SCC()
{
    uint32_t pc = PC-2;
    uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    uint8_t    eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t     eareg = (currentOpcode & 0x0007);
    uint8_t data;
    uint16_t calcTime = 14; // in case of default:

    if((this->*ConditionalTests[condition])())
        data = 0xFF;
    else
        data = 0x00;

    SetByte(eamode, eareg, calcTime, data);

    instructionsBuffer.push_back(std::to_string(pc) + "\tSCC");

    return calcTime;
}

uint16_t SCC68070::Stop() // Not fully emulated
{
    uint32_t pc = PC-2;
    uint16_t data = GetNextWord();
    uint16_t calcTime = 17;

    if(GetS())
    {
        stop = true;
        SR = data;
        if(data == 0)
            SetZ();
        else
            SetZ(0);

        if(data & 0x8000)
            SetN();
        else
            SetN(0);

        SetV(0);
        SetC(0);
    }
    else
        Exception(8, calcTime);

    instructionsBuffer.push_back(std::to_string(pc) + "\tSTOP");

    return calcTime;
}

uint16_t SCC68070::Sub()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 0;

    if(opmode == 0)
    {
        int8_t dst = D[reg] & 0x000000FF;
        int8_t src = GetByte(eamode, eareg, calcTime);
        int16_t res = dst - src;
        uint16_t ures = (uint8_t)dst - (uint8_t)res;

        if(ures & 0x100) SetXC(); else SetXC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] &= 0xFFFFFF00;
        D[reg] |= (uint8_t)res;
    }
    else if(opmode == 1)
    {
        int16_t dst = D[reg] & 0x0000FFFF;
        int16_t src = GetWord(eamode, eareg, calcTime);
        int32_t res = dst - src;
        uint32_t ures = (uint16_t)dst - (uint16_t)res;

        if(ures & 0x10000) SetXC(); else SetXC(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] &= 0xFFFF0000;
        D[reg] |= (uint16_t)res;
    }
    else if(opmode == 2)
    {
        int32_t dst = D[reg];
        int32_t src = GetLong(eamode, eareg, calcTime);
        int64_t res = dst - src;
        uint64_t ures = (uint32_t)dst - (uint32_t)res;

        if(ures & 0x100000000) SetXC(); else SetXC(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] = res;
    }
    else if(opmode == 4)
    {
        int8_t dst = GetByte(eamode, eareg, calcTime);
        int8_t src = D[reg] & 0x000000FF;
        int16_t res = dst - src;
        uint16_t ures = (uint8_t)dst - (uint8_t)src;

        if(ures & 0x100) SetXC(); else SetXC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        calcTime += 11;
        SetByte(lastAddress, res);
    }
    else if(opmode == 5)
    {
        int16_t dst = GetByte(eamode, eareg, calcTime);
        int16_t src = D[reg] & 0x0000FFFF;
        int32_t res = dst - src;
        uint32_t ures = (uint16_t)dst - (uint16_t)src;

        if(ures & 0x10000) SetXC(); else SetXC(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        calcTime += 11;
        SetWord(lastAddress, res);
    }
    else
    {
        int32_t dst = GetLong(eamode, eareg, calcTime);
        int32_t src = D[reg];
        int64_t res = dst - src;
        uint64_t ures = (uint32_t)dst - (uint32_t)src;

        if(ures & 0x100000000) SetXC(); else SetXC(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        calcTime += 15;
        SetLong(lastAddress, res);
    }

    return calcTime;
}

uint16_t SCC68070::Suba()
{
    uint32_t pc = PC-2;
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x0100) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;
    int32_t src;

    if(opmode) // Long
        src = GetLong(eamode, eareg, calcTime);
    else // Word
        src = signExtend16(GetWord(eamode, eareg, calcTime));

    A[reg] -= src;
    instructionsBuffer.push_back(std::to_string(pc) + "\tSUBA");
    return calcTime;
}

uint16_t SCC68070::Subi()
{
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 0;

    if(size == 0) // Byte
    {
        int8_t data = GetNextWord() & 0x00FF;
        int8_t dst = GetByte(eamode, eareg, calcTime);
        int16_t res = dst - data;
        uint16_t ures = (uint8_t)dst - (uint8_t)data;

        if(ures & 0x100) SetXC(); else SetXC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        if(eamode)
        {   SetByte(lastAddress, res); calcTime += 18; }
        else
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= (uint8_t)res; calcTime = 14; }
    }
    else if(size == 1) // Word
    {
        int16_t data = GetNextWord();
        int16_t dst = GetWord(eamode, eareg, calcTime);
        int32_t res = dst - data;
        uint32_t ures = (uint16_t)dst - (uint16_t)data;

        if(ures & 0x10000) SetXC(); else SetXC(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        if(eamode)
        {   SetWord(lastAddress, res); calcTime += 18; }
        else
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= (uint16_t)res; calcTime = 14; }
    }
    else // Long
    {
        int32_t data = (GetNextWord() << 16) | GetNextWord();
        int32_t dst = GetLong(eamode, eareg, calcTime);
        int64_t res = dst - data;
        uint64_t ures = (uint32_t)dst - (uint32_t)data;

        if(ures & 0x100000000) SetXC(); else SetXC(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        if(eamode)
        {   SetLong(lastAddress, res); calcTime += 26; }
        else
        {   D[eareg] = (int32_t)res; calcTime = 18; }
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tSUBI");
    return calcTime;
}

uint16_t SCC68070::Subq()
{
    uint32_t pc = PC-2;
    uint8_t   doto = (currentOpcode & 0x0E00) >> 9;
    uint8_t   data = doto ? doto : 8;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 0) // byte
    {
        int8_t dst = GetByte(eamode, eareg, calcTime);
        int16_t res = dst - data;
        uint16_t ures = (uint8_t)dst - data;

        if(ures & 0x100) SetXC(); else SetXC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= (uint8_t)res; }
        else
        {   SetByte(lastAddress, res); calcTime += 4; }
    }
    else if(size == 1) // word
    {
        int16_t dst = GetByte(eamode, eareg, calcTime);
        int32_t res = dst - data;
        uint32_t ures = (uint16_t)dst - data;

        if(eamode != 1)
        {
            if(ures & 0x10000) SetXC(); else SetXC(0);
            if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x8000) SetN(); else SetN(0);
        }

        if(eamode == 0)
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= (uint16_t)res; }
        else if(eamode == 1)
            A[eareg] = signExtend16(res);
        else
        {   SetWord(lastAddress, res); calcTime += 4; }
    }
    else // long
    {
        int32_t dst = GetByte(eamode, eareg, calcTime);
        int64_t res = dst - data;
        uint64_t ures = (uint32_t)dst - data;

        if(eamode != 1)
        {
            if(ures & 0x100000000) SetXC(); else SetXC(0);
            if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x80000000) SetN(); else SetN(0);
        }

        if(eamode == 0)
            D[eareg] = res;
        else if(eamode == 1)
            A[eareg] = res;
        else
        {   SetLong(lastAddress, res); calcTime += 8; }
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tSUBQ");
    return calcTime;
}

uint16_t SCC68070::Subx()
{
    uint32_t pc = PC-2;
    uint8_t   ry = (currentOpcode & 0x0E00) >> 9;
    uint8_t size = (currentOpcode & 0x00C0) >> 6;
    uint8_t   rm = (currentOpcode & 0x0008) >> 3;
    uint8_t   rx = (currentOpcode & 0x0007);
    uint16_t calcTime;

    if(rm) // memory to memory
    {
        if(size == 0) // byte
        {
            int8_t src = GetByte(ARIWPr(rx, 1));
            int8_t dst = GetByte(ARIWPr(ry, 1));
            int16_t res = dst - src - GetX();
            uint16_t ures = (uint8_t)dst - (uint8_t)src - GetX();

            if(ures & 0x100) SetXC(); else SetXC(0);
            if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x80) SetN(); else SetN(0);

            SetByte(lastAddress, res);
            calcTime = 28;
        }
        else if(size == 1) // word
        {
            int16_t src = GetWord(ARIWPr(rx, 2));
            int16_t dst = GetWord(ARIWPr(ry, 2));
            int32_t res = dst - src - GetX();
            uint32_t ures = (uint16_t)dst - (uint16_t)src - GetX();

            if(ures & 0x10000) SetXC(); else SetXC(0);
            if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x8000) SetN(); else SetN(0);

            SetWord(lastAddress, res);
            calcTime = 28;
        }
        else // long
        {
            int32_t src = GetLong(ARIWPr(rx, 4));
            int32_t dst = GetLong(ARIWPr(ry, 4));
            int64_t res = dst - src - GetX();
            uint64_t ures = (uint32_t)dst - (uint32_t)src - GetX();

            if(ures & 0x100000000) SetXC(); else SetXC(0);
            if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x80000000) SetN(); else SetN(0);

            SetLong(lastAddress, res);
            calcTime = 40;
        }
    }
    else // data reg to data reg
    {
        if(size == 0) // byte
        {
            int8_t src = D[rx] & 0x000000FF;
            int8_t dst = D[ry] & 0x000000FF;
            int16_t res = dst - src - GetX();
            uint16_t ures = (uint8_t)dst - (uint8_t)src - GetX();

            if(ures & 0x100) SetXC(); else SetXC(0);
            if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x80) SetN(); else SetN(0);

            D[ry] &= 0xFFFFFF00;
            D[ry] |= (uint8_t)res;
            calcTime = 7;
        }
        else if(size == 1) // word
        {
            int16_t src = D[rx] & 0x0000FFFF;
            int16_t dst = D[ry] & 0x0000FFFF;
            int32_t res = dst - src - GetX();
            uint32_t ures = (uint16_t)dst - (uint16_t)src - GetX();

            if(ures & 0x10000) SetXC(); else SetXC(0);
            if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x8000) SetN(); else SetN(0);

            D[ry] &= 0xFFFF0000;
            D[ry] |= (uint16_t)res;
            calcTime = 7;
        }
        else // long
        {
            int32_t src = D[rx];
            int32_t dst = D[ry];
            int64_t res = dst - src - GetX();
            uint64_t ures = (uint32_t)dst - (uint32_t)src - GetX();

            if(ures & 0x100000000) SetXC(); else SetXC(0);
            if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x80000000) SetN(); else SetN(0);

            D[ry] = (int32_t)res;
            calcTime = 7;
        }
    }

    instructionsBuffer.push_back(std::to_string(pc) + "\tSUBX");
    return calcTime;
}

uint16_t SCC68070::Swap()
{
    uint32_t pc = PC-2;
    uint8_t reg = currentOpcode & 0x0007;

    uint16_t tmp = (D[reg] & 0xFFFF0000) >> 16;
    D[reg] <<= 16;
    D[reg] |= tmp;

    if(D[reg] == 0)
        SetZ();
    else
        SetZ(0);

    if(D[reg] & 0x80000000)
        SetN();
    else
        SetN(0);

    SetV(0);
    SetC(0);

    instructionsBuffer.push_back(std::to_string(pc) + "\tSWAP");

    return 7;
}

uint16_t SCC68070::Tas()
{
    uint32_t pc = PC-2;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 10;

    uint8_t data = GetByte(eamode, eareg, calcTime);
    if(data == 0)
        SetZ();
    else
        SetZ(0);

    if(data & 0x80)
        SetN();
    else
    {   SetZ(0); data |= 0x80; }

    SetV(0);
    SetC(0);

    if(eamode)
    {   calcTime++; SetByte(lastAddress, data); }
    else
    {   D[eareg] &= 0xFFFFFF00; D[eareg] |= data; }

    instructionsBuffer.push_back(std::to_string(pc) + "\tTAS");

    return calcTime;
}

uint16_t SCC68070::Trap()
{
    uint32_t pc = PC-2;
    uint8_t vec = currentOpcode & 0x000F;
    uint16_t calcTime = 0;
    Exception(32 + vec, calcTime);
    instructionsBuffer.push_back(std::to_string(pc) + "\tTRAP");
    return calcTime;
}

uint16_t SCC68070::Trapv()
{
    uint32_t pc = PC-2;
    uint16_t calcTime = 0;

    if(GetV())
        Exception(7, calcTime);
    else
        calcTime = 10;
    instructionsBuffer.push_back(std::to_string(pc) + "\tTRAPV");
    return calcTime;
}

uint16_t SCC68070::Tst()
{
#define COMPARE if(!data) SetZ(); else SetZ(0); if(data < 0) SetN(); else SetN(0);
    uint32_t pc = PC-2;
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 0)
    {
        int8_t data = GetByte(eamode, eareg, calcTime);
        COMPARE
    }
    else if(size == 1)
    {
        int16_t data = GetWord(eamode, eareg, calcTime);
        COMPARE
    }
    else
    {
        int32_t data = GetLong(eamode, eareg, calcTime);
        COMPARE
    }

    SetC(0);
    SetV(0);

    instructionsBuffer.push_back(std::to_string(pc) + "\tTST");

    return calcTime;
}

uint16_t SCC68070::Unlk()
{
    uint32_t pc = PC-2;
    uint8_t reg = currentOpcode & 0x0007;
    SP = A[reg];
    A[reg] = GetLong(ARIWPo(7, 4));
    instructionsBuffer.push_back(std::to_string(pc) + "\tUNLK");
    return 15;
}
