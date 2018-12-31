#include "SCC68070.hpp"

#include "../../utils.h"

void SCC68070::Exception(const uint8_t& vectorNumber, uint16_t& calcTime, bool longFormat)
{
    uint16_t sr = SR;
    SetS();

    if(longFormat) // TODO: implement long Stack format (please fix it)
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
    return 0;
}

uint16_t SCC68070::Abcd()
{
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

    return calcTime;
}

uint16_t SCC68070::Add()
{
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

        if(res < 0) SetN();
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

        if(res < 0) SetN();
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

        if(res < 0) SetN();
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

        if(res < 0) SetN();
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

        if(res < 0) SetN();
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

        if(res < 0) SetN();
        else SetN(0);

        SetLong(lastAddress, res);

        calcTime += 8;
    }

    return calcTime;
}

uint16_t SCC68070::Adda()
{
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

    return calcTime;
}

uint16_t SCC68070::Addi()
{
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

        if(res < 0) SetN();
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

        if(res < 0) SetN();
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

        if(res < 0) SetN();
        else SetN(0);

        if(eamode == 0)
        {   D[eareg] = res; calcTime += 4; }
        else
        {   SetLong(lastAddress, res); calcTime += 12; }
    }

    return calcTime;
}

uint16_t SCC68070::Addq()
{
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

        if(res < 0) SetN();
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

        if(res < 0) SetN();
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

        if(res < 0) SetN();
        else SetN(0);

        if(eamode == 0)
            D[eareg] = res;
        else
        {   SetByte(lastAddress, res); calcTime += 8; }
    }

    return calcTime;
}

uint16_t SCC68070::Addx()
{
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

        if(res < 0) SetN();
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

        if(res < 0) SetN();
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

        if(res < 0) SetN();
        else SetN(0);

        if(rm)
            SetLong(lastAddress, res);
        else
            D[Rx] = res;
    }

    return calcTime;
}

uint16_t SCC68070::And()
{
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
    return calcTime;
}

uint16_t SCC68070::Andi()
{
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
        uint32_t data = GetNextWord();
        uint32_t  dst = GetWord(eamode, eareg, calcTime);
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
    return calcTime;
}

uint16_t SCC68070::AsM()
{


    return 14;
}

uint16_t SCC68070::AsR()
{


    return 14;
}

uint16_t SCC68070::BCC()
{
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
            PC += (int16_t)GetNextWord();
    }

    return calcTime;
}

uint16_t SCC68070::BchgD()
{



}

uint16_t SCC68070::BchgS()
{



}

uint16_t SCC68070::Bclr()
{



}

uint16_t SCC68070::Bra()
{
    int8_t disp = currentOpcode & 0x00FF;
    uint16_t calcTime;

    if(disp)
    {
        PC += disp;
        calcTime = 13;
    }
    else
    {
        PC += (int16_t)GetNextWord();
        calcTime = 14;
    }

    return calcTime;
}

uint16_t SCC68070::Bset()
{



}

uint16_t SCC68070::Bsr()
{
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
        PC += dsp;
        calcTime = 25;
    }

    return 14;
}

uint16_t SCC68070::BtstD()
{



}

uint16_t SCC68070::BtstS()
{



}

uint16_t SCC68070::Chk()
{



}

uint16_t SCC68070::Clr()
{



}

uint16_t SCC68070::Cmp()
{



}

uint16_t SCC68070::Cmpa()
{



}

uint16_t SCC68070::Cmpi()
{



}

uint16_t SCC68070::Cmpm()
{
#ifdef DEBUG_OPCODE
    log("CMPM ");
#endif // DEBUG_OPCODE
    uint8_t SIZE = (currentOpcode & 0x00C0) >> 6;
    uint8_t Ax = (currentOpcode & 0x0E00) >> 9;
    uint8_t Bx = (currentOpcode & 0x0007);



    return (SIZE <= 1) ? 18 : 26;
}

uint16_t SCC68070::DbCC() // Program Control
{
#ifdef LOG_OPCODE
    log("DBcc");
#endif // LOG_OPCODE
    uint16_t calcTime;
    uint8_t CONDITION = (currentOpcode & 0x0F00) >> 8;
    uint8_t REGISTER = currentOpcode & 0x0007;
    uint32_t pc = PC;
    int16_t displacement = GetNextWord();



    return calcTime;
}

uint16_t SCC68070::Divs()
{



}

uint16_t SCC68070::Divu()
{



}

uint16_t SCC68070::Eor()
{



}

uint16_t SCC68070::Eori()
{



}

uint16_t SCC68070::Exg()
{



}

uint16_t SCC68070::Ext()
{



}

uint16_t SCC68070::Jmp() // Program Control
{
#ifdef LOG_OPCODE
    std::cout << "JMP ";
#endif // LOG_OPCODE
    uint8_t MODE = (currentOpcode & 0x0038) >> 3;
    uint8_t REGISTER = currentOpcode & 0x0007;
    uint16_t calcTime = 14; // arbitrary, only if // errorLog macro is used

    if(MODE == 2)
    {   PC = A[REGISTER]; calcTime = 7; }
    else if(MODE == 5)
    {   PC = ARIWD(REGISTER); calcTime = 14; }
    else if(MODE == 6)
    {   PC = ARIWI8(REGISTER); calcTime = 17; }
    else if(MODE == 7)
    {
        //PC = AM7(REGISTER);
        if(REGISTER == 0)
            calcTime = 14;
        else if(REGISTER == 1)
            calcTime = 18;
        else if(REGISTER == 2)
            calcTime = 14;
        else
            calcTime = 17;
    }
    else
        // errorLog("Wrong addressing mode in JMP instruction");

    return calcTime;
}

uint16_t SCC68070::Jsr() // Program Control
{
#ifdef LOG_OPCODE
    std::cout << "JSR ";
#endif // LOG_OPCODE
    uint8_t MODE = (currentOpcode & 0x0038) >> 3;
    uint8_t REGISTER = currentOpcode & 0x0007;
    uint32_t addr = ARIWPr(7, 4);
    uint16_t calcTime = 26; // arbitrary, only if // errorLog macro is used

    if(MODE == 2)
    {   PC = A[REGISTER]; calcTime = 18; }
    else if(MODE == 5)
    {   PC = ARIWD(REGISTER); calcTime = 25; }
    else if(MODE == 6)
    {   PC = ARIWI8(REGISTER); calcTime = 28; }
    else if(MODE == 7)
    {
//        PC = AM7(REGISTER);
        if(REGISTER == 0)
            calcTime = 25;
        else if(REGISTER == 1)
            calcTime = 29;
        else if(REGISTER == 2)
            calcTime = 25;
        else
            calcTime = 28;
    }
    else
        // errorLog("Wrong addressing mode in JMP instruction");

    SetLong(addr, PC);

    return calcTime;
}

uint16_t SCC68070::Lea()
{



}

uint16_t SCC68070::Link()
{



}

uint16_t SCC68070::LsM()
{



}

uint16_t SCC68070::LsR()
{



}

uint16_t SCC68070::Move()
{



}

uint16_t SCC68070::Moveccr()
{



}

uint16_t SCC68070::Movesr()
{



}

uint16_t SCC68070::MoveFsr()
{
#ifdef LOG_OPCODE
    std::cout << "MOVEfSR may not be used"; // According to the Green Book Chapter VI.2.2.2
#endif // LOG_OPCODE


}

uint16_t SCC68070::Moveusp()
{



}

uint16_t SCC68070::Movea()
{



}

uint16_t SCC68070::Movem()
{



}

uint16_t SCC68070::Movep()
{



}

uint16_t SCC68070::Moveq()
{



}

uint16_t SCC68070::Muls()
{



}

uint16_t SCC68070::Mulu()
{



}

uint16_t SCC68070::Nbcd()
{
    uint8_t mode = (currentOpcode & 0x0038) >> 3;
    uint8_t reg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;
    uint8_t x = GetX();
    uint8_t data = GetByte(mode, reg, calcTime);
    uint8_t result = 0 - convertPBCD(data);

    if(!(data & 0x0F))
        SetXC();

    result -= x;

    if(result != 0)
        SetZ(0);

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



}

uint16_t SCC68070::Negx()
{



}

uint16_t SCC68070::Nop() // ok
{
    return 14; // I love this instruction :D
}

uint16_t SCC68070::Not() // ok
{
#ifdef LOG_OPCODE
    log("NOT ");
#endif // LOG_OPCODE
    uint8_t size = (currentOpcode & 0x00C0) >> 6;
    uint8_t MODE = (currentOpcode & 0x0038) >> 3;
    uint8_t REGISTER = currentOpcode & 0x0007;
    uint16_t calcTime = 0; // arbitrary, set as default

    if(size == 0) // Byte
    {
        int8_t data = GetByte(MODE, REGISTER, calcTime);
        int8_t res = ~data;
        SetByte(lastAddress, res);

        if(res == 0)
            SetZ();
        else
            SetZ(0);
        if(res < 0)
            SetN();
        else
            SetN(0);
    }
    else if(size == 1) // Word
    {
        int16_t data = GetWord(MODE, REGISTER, calcTime);
        int16_t res = ~data;
        SetWord(lastAddress, res);

        if(res == 0)
            SetZ();
        else
            SetZ(0);
        if(res < 0)
            SetN();
        else
            SetN(0);
    }
    else // long
    {
        int32_t data = GetLong(MODE, REGISTER, calcTime);
        int32_t res = ~data;
        SetLong(lastAddress, res);

        if(res == 0)
            SetZ();
        else
            SetZ(0);
        if(res < 0)
            SetN();
        else
            SetN(0);
    }
    SetVC(0);

    if(MODE <= 1)
        calcTime += 7;
    else
        if(size <= 1)
            calcTime += 11;
        else
            calcTime += 15;

    return calcTime;
}

uint16_t SCC68070::Or()
{



}

uint16_t SCC68070::Ori()
{



}

uint16_t SCC68070::Pea()
{



}

uint16_t SCC68070::Reset()
{
#ifdef LOG_OPCODE
    log("RESET ");
#endif // LOG_OPCODE

    return 154;
}

uint16_t SCC68070::RoM()
{



}

uint16_t SCC68070::RoR()
{



}

uint16_t SCC68070::RoxM()
{
#ifdef LOG_OPCODE
    std::cout << "ROXm";
#endif // LOG_OPCODE


}

uint16_t SCC68070::RoxR()
{
#ifdef LOG_OPCODE
    std::cout << "ROXr";
#endif // LOG_OPCODE


}

uint16_t SCC68070::Rte()
{



}

uint16_t SCC68070::Rtr() // Program Control
{
#ifdef LOG_OPCODE
    log("RTR ");
#endif // LOG_OPCODE

    return 22;
}

uint16_t SCC68070::Rts() // Program Control
{
#ifdef LOG_OPCODE
    log("RTS ");
#endif // LOG_OPCODE

    return 15;
}

uint16_t SCC68070::Sbcd()
{
    uint8_t Ry = (currentOpcode & 0x0E00) >> 9;
    uint8_t Rx = (currentOpcode & 0x0007);
    uint8_t result = 0;
    uint16_t calcTime;
    uint8_t x = GetX();

    if(currentOpcode & 0x0008) // memory to memory
    {
        uint8_t src = GetByte(ARIWPr(Rx, 1));
        uint8_t dst = GetByte(ARIWPr(Ry, 1));
        if((dst & 0x0F) > (src & 0x0F))
            SetXC();
        if(((dst & 0xF0) >> 4) > ((src & 0xF0) >> 4))
            SetXC();

        result = convertPBCD(dst) - convertPBCD(src) - x;
        SetByte(lastAddress, result);
        calcTime = 31;
    }
    else
    {
        uint8_t src = D[Rx] & 0x000000FF;
        uint8_t dst = D[Ry] & 0x000000FF;
        if((dst & 0x0F) > (src & 0x0F))
            SetXC();
        if(((dst & 0xF0) >> 4) > ((src & 0xF0) >> 4))
            SetXC();

        result = convertPBCD(dst) - convertPBCD(src) - x;
        D[Ry] &= 0xFFFFFF00;
        D[Ry] |= result;
        calcTime = 10;
    }

    if(result != 0)
        SetZ(0);

    return calcTime;
}

uint16_t SCC68070::SCC() // Program Control
{
#ifdef LOG_OPCODE
    log("Scc ");
#endif // LOG_OPCODE
    uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    uint8_t MODE      = (currentOpcode & 0x0038) >> 3;
    uint8_t REGISTER  = (currentOpcode & 0x0007);
    uint8_t data;
    uint32_t addr = 0;
    uint16_t calcTime = 14; // in case of default:

    if((this->*ConditionalTests[condition])())
        data = 0xFF;
    else
        data = 0x00;

    switch(MODE)
    {
    case 0:
        D[REGISTER] &= 0xFFFFFF00;
        D[REGISTER] |= data;
        calcTime = 13;
    break;
    case 2:
        SetByte(A[REGISTER], data);
        calcTime = 17 + ITARIBW;
    break;
    case 3:
        addr = ARIWPo(REGISTER, 1);
        SetByte(addr, data);
        calcTime = 17 + ITARIBW;
    break;
    case 4:
        addr = ARIWPr(REGISTER, 1);
        SetByte(addr, data);
        calcTime = 17 + ITARIBW;
    break;
    case 5:
        addr = ARIWD(REGISTER);
        SetByte(addr, data);
        calcTime = 17 + ITARIBW;
    break;
    case 6:
        addr = ARIWI8(REGISTER);
        SetByte(addr, data);
        calcTime = 17 + ITARIBW;
    break;
    case 7:
        if(!REGISTER)
        {
            addr = AbsoluteShortAddressing();
            SetByte(addr, data);
        calcTime = 17 + ITARIBW;
        }
        else if(REGISTER == 1)
        {
            addr = AbsoluteLongAddressing();
            SetByte(addr, data);
            calcTime = 17 + ITARIBW;
        }
        else
            // errorLog("Wrong Regsiter for addressing mode 7 in Scc " << REGISTER);
    break;
        // errorLog("Wrong addressing mode for Scc : " << MODE);
    }

    return calcTime;
}

uint16_t SCC68070::Stop() // ok
{
#ifdef LOG_OPCODE
    log("STOP ");
#endif // LOG_OPCODE
    uint16_t data = GetNextWord(); PC += 2;
    if(GetS())
    {
        SR = data;
        if(data == 0)
            SetZ();
        else
            SetZ(0);
        if(data & 0x8000)
            SetN();
        else
            SetN(0);
    }

    return 17;
}

uint16_t SCC68070::Sub()
{



}

uint16_t SCC68070::Suba()
{



}

uint16_t SCC68070::Subi()
{



}

uint16_t SCC68070::Subq()
{



}

uint16_t SCC68070::Subx()
{



}

uint16_t SCC68070::Swap()
{
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
    return 7;
}

uint16_t SCC68070::Tas()
{
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
    return calcTime;
}

uint16_t SCC68070::Trap()
{
    uint8_t vec = currentOpcode & 0x000F;
    uint16_t calcTime = 0;
    Exception(32 + vec, calcTime);
    return calcTime;
}

uint16_t SCC68070::Trapv()
{
    uint16_t calcTime = 0;

    if(GetV())
        Exception(7, calcTime);
    else
        calcTime = 10;

    return calcTime;
}

uint16_t SCC68070::Tst()
{
#define COMPARE if(!data) SetZ(); else SetZ(0); if(data < 0) SetN(); else SetN(0);
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
    return calcTime;
}

uint16_t SCC68070::Unlk()
{
    uint8_t reg = currentOpcode & 0x0007;
    SP = A[reg];
    A[reg] = GetLong(ARIWPo(7, 4));
    return 15;
}
