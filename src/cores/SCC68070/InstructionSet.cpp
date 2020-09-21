#include "SCC68070.hpp"

#include "../../utils.hpp"

uint16_t SCC68070::Exception(const uint8_t vectorNumber)
{
    uint16_t calcTime = 0;
    uint16_t sr = SR;
    SetS();
    disassembledInstructions.push_back("Exception vector " + std::to_string(vectorNumber) + ": " + DisassembleException(vectorNumber));

    if(vectorNumber == 2 || vectorNumber == 3) // TODO: implement long Stack format
    {
        uint32_t last = lastAddress;
        SetWord(ARIWPr(7, 2), 0); // internal information
        SetWord(ARIWPr(7, 2), currentOpcode); // IRC
        SetWord(ARIWPr(7, 2), currentOpcode); // IR
        SetLong(ARIWPr(7, 4), 0); // DBIN
        SetLong(ARIWPr(7, 4), last); // TPF
        SetLong(ARIWPr(7, 4), 0); // TPD
        SetWord(ARIWPr(7, 2), 0); // internal information
        SetWord(ARIWPr(7, 2), 0); // internal information
        SetWord(ARIWPr(7, 2), 0); // Current Move Multiple Mask
        SetWord(ARIWPr(7, 2), 0); // Special Status Word
        SetWord(ARIWPr(7, 2), 0xF000 | ((uint16_t)vectorNumber << 2));
    }
    else
        SetWord(ARIWPr(7, 2), (uint16_t)vectorNumber << 2);

    SetLong(ARIWPr(7, 4), PC);
    SetWord(ARIWPr(7, 2), sr);

    if(vectorNumber <= 1 || vectorNumber == 9 || vectorNumber == 24)
        loop = true;

    switch(vectorNumber) // handle Exception Processing Clock Periods
    {
    case BusError: case AddressError:
        calcTime += 158; break;
    case IllegalInstruction: case PrivilegeViolation: case Trace: case TRAPVInstruction:
        calcTime += 55; break;
    case 32: case 33: case 34: case 35: case 36: case 37: case 38: case 39:
    case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47:
        calcTime += 52; break;
    case ZeroDivide:
        calcTime += 64; break;
    default:
        if(vectorNumber == 15 || (vectorNumber >= 24 && vectorNumber < 32) || vectorNumber >= 57)
            calcTime += 65;
    }

    PC = GetLong(vectorNumber * 4);
    return calcTime;
}

uint16_t SCC68070::UnknownInstruction()
{
    throw SCC68070Exception(IllegalInstruction);
}

uint16_t SCC68070::ABCD()
{
    uint8_t Rx = (currentOpcode & 0x0E00) >> 9;
    uint8_t Ry = (currentOpcode & 0x0007);
    uint8_t rm = currentOpcode & 0x0008;
    uint16_t calcTime;
    uint8_t result = 0;
    uint8_t x = GetX();

    if(rm) // Memory to Memory
    {
        uint8_t src = PBCDToByte(GetByte(ARIWPr(Ry, 1))) + x;
        uint8_t dst = PBCDToByte(GetByte(ARIWPr(Rx, 1)));

        if(src + dst >= 100) SetXC(); else SetXC(0);
        result = byteToPBCD(src + dst);

        SetByte(lastAddress, result);
        calcTime = 31;
    }
    else // Data register to Data register
    {
        uint8_t src = PBCDToByte(D[Ry] & 0x000000FF) + x;
        uint8_t dst = PBCDToByte(D[Rx] & 0x000000FF);

        if(src + dst >= 100) SetXC(); else SetXC(0);
        result = byteToPBCD(src + dst);

        D[Rx] &= 0xFFFFFF00;
        D[Rx] |= result;
        calcTime = 10;
    }

    if(result != 0)
        SetZ(0);

    return calcTime;
}

uint16_t SCC68070::ADD()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(opmode == 0)
    {
        int8_t src = GetByte(eamode, eareg, calcTime);
        int8_t dst = D[reg] & 0x000000FF;
        int16_t res = src + dst;
        uint16_t ures = (uint8_t)src + (uint8_t)dst;

        if(ures & 0x100) SetXC(); else SetXC(0);
        if(res > INT8_MAX || res < INT8_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        D[reg] &= 0xFFFFFF00;
        D[reg] |= (res & 0xFF);
    }
    else if(opmode == 1)
    {
        int16_t src = GetWord(eamode, eareg, calcTime);
        int16_t dst = D[reg] & 0x0000FFFF;
        int32_t res = src + dst;
        uint32_t ures = (uint16_t)src + (uint16_t)dst;

        if(ures & 0x10000) SetXC(); else SetXC(0);
        if(res > INT16_MAX || res < INT16_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        D[reg] &= 0xFFFF0000;
        D[reg] |= (res & 0xFFFF);
    }
    else if(opmode == 2)
    {
        int32_t src = GetLong(eamode, eareg, calcTime);
        int32_t dst = D[reg];
        int64_t res = src + dst;
        uint64_t ures = (uint32_t)src + (uint32_t)dst;

        if(ures & 0x100000000) SetXC(); else SetXC(0);
        if(res > INT32_MAX || res < INT32_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        D[reg] = res;
    }
    else if(opmode == 4)
    {
        int8_t src = D[reg] & 0x000000FF;
        int8_t dst = GetByte(eamode, eareg, calcTime);
        int16_t res = src + dst;
        uint16_t ures = (uint8_t)src + (uint8_t)dst;

        if(ures & 0x100) SetXC(); else SetXC(0);
        if(res > INT8_MAX || res < INT8_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        SetByte(lastAddress, res);
        calcTime += 4;
    }
    else if(opmode == 5)
    {
        int16_t src = D[reg] & 0x0000FFFF;
        int16_t dst = GetWord(eamode, eareg, calcTime);
        int32_t res = src + dst;
        uint32_t ures = (uint16_t)src + (uint16_t)dst;

        if(ures & 0x10000) SetXC(); else SetXC(0);
        if(res > INT16_MAX || res < INT16_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        SetWord(lastAddress, res);
        calcTime += 4;
    }
    else
    {
        int32_t src = D[reg];
        int32_t dst = GetLong(eamode, eareg, calcTime);
        int64_t res = src + dst;
        uint64_t ures = (uint32_t)src + (uint32_t)dst;

        if(ures & 0x100000000) SetXC(); else SetXC(0);
        if(res > INT32_MAX || res < INT32_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        SetLong(lastAddress, res);
        calcTime += 8;
    }

    return calcTime;
}

uint16_t SCC68070::ADDA()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t   size = (currentOpcode & 0x0100) >> 8;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size) // Long
    {
        int32_t src = GetLong(eamode, eareg, calcTime);
        A[reg] += src;
        if(eamode > 1)
            calcTime += 8;
    }
    else // Word
    {
        int16_t src = GetWord(eamode, eareg, calcTime);
        int16_t dst = A[reg] & 0x0000FFFF;
        A[reg] = src + dst;
        if(eamode > 1)
            calcTime += 4;
    }

    return calcTime;
}

uint16_t SCC68070::ADDI()
{
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;

    if(size == 0) // Byte
    {
        int8_t src = GetNextWord() & 0x00FF;
        int8_t dst = GetByte(eamode, eareg, calcTime);
        int16_t res = src + dst;
        uint16_t ures = (uint8_t)src + (uint8_t)dst;

        if(ures & 0x100) SetXC(); else SetXC(0);
        if(res > INT8_MAX || res < INT8_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

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

        if(ures & 0x10000) SetXC(); else SetXC(0);
        if(res > INT16_MAX || res < INT16_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

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

        if(ures & 0x100000000) SetXC(); else SetXC(0);
        if(res > INT32_MAX || res < INT32_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        if(eamode == 0)
        {   D[eareg] = res; calcTime += 4; }
        else
        {   SetLong(lastAddress, res); calcTime += 12; }
    }

    return calcTime;
}

uint16_t SCC68070::ADDQ()
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

        if(ures & 0x100) SetXC(); else SetXC(0);
        if(res > INT8_MAX || res < INT8_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= (res & 0xFF); }
        else
        {   SetByte(lastAddress, ures); calcTime += 4; }
    }
    else if(size == 1)
    {
        int16_t dst = GetWord(eamode, eareg, calcTime);
        int32_t res = data + dst;
        uint32_t ures = data + (uint16_t)dst;

        if(ures & 0x10000) SetXC(); else SetXC(0);
        if(res > INT16_MAX || res < INT16_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= (res & 0xFFFF); }
        else
        {   SetWord(lastAddress, ures); calcTime += 4; }
    }
    else
    {
        int32_t dst = GetLong(eamode, eareg, calcTime);
        int64_t res = data + dst;
        uint64_t ures = data + (uint32_t)dst;

        if(ures & 0x100000000) SetXC(); else SetXC(0);
        if(res > INT32_MAX || res < INT32_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        if(eamode == 0)
            D[eareg] = ures;
        else
        {   SetLong(lastAddress, ures); calcTime += 8; }
    }

    return calcTime;
}

uint16_t SCC68070::ADDX()
{
    uint8_t   Rx = (currentOpcode & 0x0E00) >> 9;
    uint8_t   Ry = (currentOpcode & 0x0007);
    uint8_t size = (currentOpcode & 0x00C0) >> 6;
    uint8_t   rm = (currentOpcode & 0x0008) >> 3;
    uint16_t calcTime = 7;

    if(size == 0)
    {
        int8_t src;
        int8_t dst;
        if(rm)
        {   src = GetByte(ARIWPr(Ry, 1));
            dst = GetByte(ARIWPr(Rx, 1)); calcTime = 28; }
        else
        {   src = D[Ry] & 0x000000FF;
            dst = D[Rx] & 0x000000FF; }

        int16_t res = src + dst + GetX();
        uint16_t ures = (uint8_t)src + (uint8_t)dst + GetX();

        if(ures & 0x100) SetXC(); else SetXC(0);
        if(res > INT8_MAX || res < INT8_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        if(rm)
            SetByte(lastAddress, res);
        else
        {   D[Rx] &= 0xFFFFFF00; D[Rx] |= (res & 0xFF); }
    }
    else if(size == 1)
    {
        int16_t src;
        int16_t dst;
        if(rm)
        {   src = GetWord(ARIWPr(Ry, 2));
            dst = GetWord(ARIWPr(Rx, 2)); calcTime = 28; }
        else
        {   src = D[Ry] & 0x0000FFFF;
            dst = D[Rx] & 0x0000FFFF; }

        int32_t res = src + dst + GetX();
        uint32_t ures = (uint16_t)src + (uint16_t)dst + GetX();

        if(ures & 0x10000) SetXC(); else SetXC(0);
        if(res > INT16_MAX || res < INT16_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        if(rm)
            SetWord(lastAddress, res);
        else
        {   D[Rx] &= 0xFFFF0000; D[Rx] |= (res & 0xFFFF); }
    }
    else
    {
        int32_t src;
        int32_t dst;
        if(rm)
        {   src = GetLong(ARIWPr(Ry, 4));
            dst = GetLong(ARIWPr(Rx, 4)); calcTime = 40; }
        else
        {   src = D[Ry];
            dst = D[Rx]; }

        int64_t res = src + dst + GetX();
        uint64_t ures = (uint32_t)src + (uint32_t)dst + GetX();

        if(ures & 0x100000000) SetXC(); else SetXC(0);
        if(res > INT32_MAX || res < INT32_MIN) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        if(rm)
            SetLong(lastAddress, res);
        else
            D[Rx] = res;
    }

    return calcTime;
}

uint16_t SCC68070::AND()
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

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        D[reg] &= 0xFFFFFF00;
        D[reg] |= res;
    }
    else if(opmode == 1)
    {
        uint16_t src = GetWord(eamode, eareg, calcTime);
        uint16_t dst = D[reg] & 0x0000FFFF;
        uint16_t res = src & dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        D[reg] &= 0xFFFF0000;
        D[reg] |= res;
    }
    else if(opmode == 2)
    {
        uint32_t src = GetLong(eamode, eareg, calcTime);
        uint32_t dst = D[reg];
        uint32_t res = src & dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        D[reg] = res;
    }
    else if(opmode == 4)
    {
        uint8_t src = D[reg] & 0x000000FF;
        uint8_t dst = GetByte(eamode, eareg, calcTime);
        uint8_t res = src & dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        calcTime += 4;
        SetByte(lastAddress, res);
    }
    else if(opmode == 5)
    {
        uint16_t src = D[reg] & 0x0000FFFF;
        uint16_t dst = GetWord(eamode, eareg, calcTime);
        uint16_t res = src & dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        calcTime += 4;
        SetWord(lastAddress, res);
    }
    else
    {
        uint32_t src = D[reg];
        uint32_t dst = GetLong(eamode, eareg, calcTime);
        uint32_t res = src & dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        calcTime += 8;
        SetLong(lastAddress, res);
    }
    SetVC(0);

    return calcTime;
}

uint16_t SCC68070::ANDI()
{
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;

    if(size == 0)
    {
        uint8_t data = GetNextWord() & 0x00FF;
        uint8_t  dst = GetByte(eamode, eareg, calcTime);
        uint8_t  res = data & dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

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

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

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

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        if(eamode)
        {   SetLong(lastAddress, res); calcTime += 12; }
        else
        {   D[eareg] = res; calcTime += 4; }
    }
    SetVC(0);

    return calcTime;
}

uint16_t SCC68070::ANDICCR()
{
    uint8_t data = GetNextWord() & 0x1F;
    SR |= data;
    return 14;
}

uint16_t SCC68070::ANDISR()
{
    uint16_t data = GetNextWord();

    if(!GetS())
        return Exception(PrivilegeViolation);

    SR |= data;
    return 14;
}

uint16_t SCC68070::ASm()
{
    uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;
    uint16_t data = GetWord(eamode, eareg, calcTime);

    bool b = false;
    if(dr) // left
    {
        uint16_t a = data & 0x8000;
        SetXC(a ? 1 : 0);
        data <<= 1;
        if(a != (data & 0x8000))
            b = true;
    }
    else // right
    {
        uint16_t msb = data & 0x8000;
        uint16_t a = data & 0x0001;
        SetXC(a);
        data >>= 1;
        data |= msb;
    }

    if(data & 0x8000) SetN(); else SetN(0);
    if(data == 0) SetZ(); else SetZ(0);
    if(b) SetV(); else SetV(0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::ASr()
{
    uint8_t count = (currentOpcode & 0x0E00) >> 9;
    uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    uint8_t   reg = (currentOpcode & 0x0007);
    uint8_t shift;

    if(ir)
        shift = D[count] % 64;
    else
        shift = (count) ? count : 8;

    if(!shift)
    {
        SetV(0);
        SetC(0);
        if(size == 0)
        {
            uint8_t data = D[reg] & 0x000000FF;
            if(data & 0x80) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
        else if(size == 1)
        {
            uint16_t data = D[reg] & 0x0000FFFF;
            if(data & 0x8000) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
        else
        {
            uint32_t data = D[reg];
            if(data & 0x80000000) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
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

    return 13 + 3 * shift;
}

uint16_t SCC68070::Bcc()
{
    uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    int16_t      disp = (int8_t)(currentOpcode & 0x00FF);
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
        disp = GetNextWord();
        if((this->*ConditionalTests[condition])())
            PC += disp - 2;
    }

    return calcTime;
}

uint16_t SCC68070::BCHG()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 10;
    uint8_t shift;

    if(currentOpcode & 0x0100)
    {
        shift = D[reg] % (eamode ? 8 : 32);
        if(eamode)
            calcTime += 4;
    }
    else
    {
        shift = (GetNextWord() & 0x00FF) % (eamode ? 8 : 32);
        if(eamode)
            calcTime += 11;
        else
            calcTime += 7;
    }

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

    return calcTime;
}

uint16_t SCC68070::BCLR()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 10;
    uint8_t shift;

    if(currentOpcode & 0x0100)
    {
        shift = D[reg] % (eamode ? 8 : 32);
        if(eamode)
            calcTime += 4;
    }
    else
    {
        shift = (GetNextWord() & 0x00FF) % (eamode ? 8 : 32);
        if(eamode)
            calcTime += 11;
        else
            calcTime += 7;
    }

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
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t mask = 1 << shift;

        if(data & mask)
            SetZ(0);
        else
            SetZ();

        data &= ~(mask);
        SetByte(lastAddress, data);
    }

    return calcTime;
}

uint16_t SCC68070::BRA()
{
    int16_t disp = (int8_t)(currentOpcode & 0x00FF);
    uint16_t calcTime;

    if(disp)
    {
        PC += disp;
        calcTime = 13;
    }
    else
    {
        disp = GetNextWord();
        PC += disp - 2;
        calcTime = 14;
    }

    return calcTime;
}

uint16_t SCC68070::BSET()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 10;
    uint8_t shift;

    if(currentOpcode & 0x0100)
    {
        shift = D[reg] % (eamode ? 8 : 32);
        if(eamode)
            calcTime += 4;
    }
    else
    {
        shift = (GetNextWord() & 0x00FF) % (eamode ? 8 : 32);
        if(eamode)
            calcTime += 11;
        else
            calcTime += 7;
    }

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
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t mask = 1 << shift;

        if(data & mask)
            SetZ(0);
        else
            SetZ();

        data |= mask;
        SetByte(lastAddress, data);
    }

    return calcTime;
}

uint16_t SCC68070::BSR()
{
    int16_t disp = (int8_t)(currentOpcode & 0x00FF);
    uint16_t calcTime;

    if(disp)
    {
        SetLong(ARIWPr(7, 4), PC);
        PC += disp;
        calcTime = 17;
    }
    else
    {
        disp = GetNextWord();
        SetLong(ARIWPr(7, 4), PC);
        PC += disp - 2;
        calcTime = 22;
    }

    return calcTime;
}

uint16_t SCC68070::BTST()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;
    uint8_t shift;

    if(currentOpcode & 0x0100)
    {
        shift = D[reg] % (eamode ? 8 : 32);
    }
    else
    {
        shift = (GetNextWord() & 0x00FF) % (eamode ? 8 : 32);
        calcTime += 7;
    }

    if(eamode == 0)
    {
        uint32_t data = D[eareg];
        uint32_t mask = 1 << shift;

        if(data & mask)
            SetZ(0);
        else
            SetZ();
    }
    else
    {
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t mask = 1 << shift;

        if(data & mask)
            SetZ(0);
        else
            SetZ();
    }

    return calcTime;
}

uint16_t SCC68070::CHK()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 19;

    int16_t source = GetWord(eamode, eareg, calcTime);
    int16_t data = D[reg] & 0xFFFF;

    if(data < 0 || data > source)
    {
        calcTime += 45;
        Exception(CHKInstruction);
        if(data < 0) SetN(); else SetN(0);
    }

    return calcTime;
}

uint16_t SCC68070::CLR()
{
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

    return calcTime;
}

uint16_t SCC68070::CMP()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(opmode == 0) // Byte
    {
        int8_t src = GetByte(eamode, eareg, calcTime);
        int8_t dst = D[reg] &= 0x000000FF;
        int16_t res = dst - src;
        uint16_t ures = (uint8_t)dst - (uint8_t)src;

        if(ures & 0x100) SetC(); else SetC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);
    }
    else if(opmode == 1) // Word
    {
        int16_t src = GetWord(eamode, eareg, calcTime);
        int16_t dst = D[reg] &= 0x0000FFFF;
        int32_t res = dst - src;
        uint32_t ures = (uint16_t)dst - (uint16_t)src;

        if(ures & 0x10000) SetC(); else SetC(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);
    }
    else // Long
    {
        int32_t src = GetLong(eamode, eareg, calcTime);
        int32_t dst = D[reg];
        int64_t res = dst - src;
        uint64_t ures = (uint32_t)dst - (uint32_t)src;

        if(ures & 0x100000000) SetC(); else SetC(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);
    }

    return calcTime;
}

uint16_t SCC68070::CMPA()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x0100) >> 8;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    int32_t src;
    int32_t dst;
    int64_t res;
    uint64_t ures;

    if(opmode) // Long
    {
        src = GetLong(eamode, eareg, calcTime);
        dst = A[reg];
        res = dst - src;
        ures = (uint32_t)dst - (uint32_t)src;
    }
    else // Word
    {
        src = signExtend16(GetWord(eamode, eareg, calcTime));
        dst = signExtend16(A[reg] &= 0x0000FFFF);
        res = dst - src;
        ures = (uint32_t)dst - (uint32_t)src;
    }

    if(ures & 0x100000000) SetC(); else SetC(0);
    if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
    if(res == 0) SetZ(); else SetZ(0);
    if(res & 0x80000000) SetN(); else SetN(0);

    return calcTime;
}

uint16_t SCC68070::CMPI()
{
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
        int32_t dst = GetLong(eamode, eareg, calcTime);
        int64_t res = dst - data;
        uint64_t ures = (uint32_t)dst - (uint32_t)data;

        if(ures & 0x100000000) SetC(); else SetC(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);
    }

    return (size == 2) ? calcTime + 4 : calcTime;
}

uint16_t SCC68070::CMPM()
{
    uint8_t size = (currentOpcode & 0x00C0) >> 6;
    uint8_t   Ax = (currentOpcode & 0x0E00) >> 9;
    uint8_t   Ay = (currentOpcode & 0x0007);

    if(size == 0) // byte
    {
        int8_t src = GetByte(ARIWPo(Ay, 1));
        int8_t dst = GetByte(ARIWPo(Ax, 1));
        int16_t res = dst - src;

        if(res == 0) SetZ(); else SetZ(0);
        if(res < 0) SetN(); else SetN(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if((dst < src) ^ (((dst ^ src) >= 0) == false)) SetC(); else SetC(0); // From bizhawk : C = ((a < b) ^ ((a ^ b) >= 0) == false);
    }
    else if(size == 1) // word
    {
        int16_t src = GetWord(ARIWPo(Ay, 2));
        int16_t dst = GetWord(ARIWPo(Ax, 2));
        int32_t res = dst - src;

        if(res == 0) SetZ(); else SetZ(0);
        if(res < 0) SetN(); else SetN(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if((dst < src) ^ (((dst ^ src) >= 0) == false)) SetC(); else SetC(0);
    }
    else // long
    {
        int32_t src = GetLong(ARIWPo(Ay, 4));
        int32_t dst = GetLong(ARIWPo(Ax, 4));
        int64_t res = dst - src;

        if(res == 0) SetZ(); else SetZ(0);
        if(res < 0) SetN(); else SetN(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if((dst < src) ^ (((dst ^ src) >= 0) == false)) SetC(); else SetC(0);
    }

    return (size == 2) ? 26 : 18;
}

uint16_t SCC68070::DBcc()
{
    uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    uint8_t reg = (currentOpcode & 0x0007);
    int16_t disp = GetNextWord();

    if((this->*ConditionalTests[condition])())
       return 14;

    int16_t data = D[reg] & 0x0000FFFF;
    --data;
    D[reg] &= 0xFFFF0000;
    D[reg] |= (uint16_t)data;
    if(data == -1)
        return 17;

    PC += signExtend16(disp) - 2;

    return 17;
}

uint16_t SCC68070::DIVS()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 169; // LMAO

    int16_t src = GetWord(eamode, eareg, calcTime);
    int32_t dst = D[reg];

    if(src == 0)
        return 7 + Exception(ZeroDivide); // arbitrary

    int32_t quotient = dst / src;

    if(quotient > INT16_MAX || quotient < INT16_MIN)
    {
        SetV();
        return calcTime;
        // Considering that a quotient overflow interrupts the instruction early,
        // the calculation time may not be accurate here
    }
    else
        SetV(0);

    int16_t remainder = dst % src;
    D[reg] = ((uint16_t)remainder << 16) | (uint16_t)quotient;

    if(quotient & 0x8000) SetN(); else SetN(0);
    if(quotient == 0) SetZ(); else SetZ(0);

    SetC(0);
    return calcTime;
}

uint16_t SCC68070::DIVU()
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 130;

    uint16_t src = GetWord(eamode, eareg, calcTime);
    uint32_t dst = D[reg];

    if(src == 0)
        return 7 + Exception(ZeroDivide); // arbitrary

    uint32_t quotient = dst / src;

    if(quotient > UINT16_MAX)
    {
        SetV();
        return calcTime;
        // Considering that a quotient overflow interrupts the instruction early,
        // the calculation time may not be accurate here
    }
    else
        SetV(0);

    uint16_t remainder = dst % src;
    D[reg] = (remainder << 16) | (uint16_t)quotient;

    if(quotient & 0x8000) SetN(); else SetN(0);
    if(quotient == 0) SetZ(); else SetZ(0);

    SetC(0);
    return calcTime;
}

uint16_t SCC68070::EOR()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x001C) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(opmode == 4)
    {
        uint8_t src = D[reg] & 0x000000FF;
        uint8_t dst = GetByte(eamode, eareg, calcTime);
        uint8_t res = src ^ dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        calcTime += eamode ? 4 : 0;
        SetByte(lastAddress, res);
    }
    else if(opmode == 5)
    {
        uint16_t src = D[reg] & 0x0000FFFF;
        uint16_t dst = GetWord(eamode, eareg, calcTime);
        uint16_t res = src ^ dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        calcTime += eamode ? 4 : 0;
        SetWord(lastAddress, res);
    }
    else
    {
        uint32_t src = D[reg];
        uint32_t dst = GetLong(eamode, eareg, calcTime);
        uint32_t res = src ^ dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        calcTime += eamode ? 8 : 0;
        SetLong(lastAddress, res);
    }
    SetVC(0);

    return calcTime;
}

uint16_t SCC68070::EORI()
{
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

    return calcTime;
}

uint16_t SCC68070::EORICCR()
{
    uint8_t data = GetNextWord() & 0x1F;
    SR ^= data;
    return 14;
}

uint16_t SCC68070::EORISR()
{
    uint16_t data = GetNextWord();

    if(!GetS())
        return Exception(PrivilegeViolation);

    SR ^= data;
    return 14;
}

uint16_t SCC68070::EXG()
{
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

    return 13;
}

uint16_t SCC68070::EXT()
{
    uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    uint8_t    reg = (currentOpcode & 0x0007);

    if(opmode == 2) // byte to word
    {
        uint16_t tmp = signExtend816(D[reg] & 0x000000FF);
        D[reg] &= 0xFFFF0000;
        D[reg] |= tmp;

        if(tmp & 0x8000) SetN(); else SetN(0);
        if(tmp == 0) SetZ(); else SetZ(0);
    }
    else // word to long
    {
        D[reg] = signExtend16(D[reg] & 0x0000FFFF);

        if(D[reg] & 0x80000000) SetN(); else SetN(0);
        if(D[reg] == 0) SetZ(); else SetZ(0);
    }

    SetVC(0);

    return 7;
}

uint16_t SCC68070::ILLEGAL()
{
    throw SCC68070Exception(IllegalInstruction);
}

uint16_t SCC68070::JMP()
{
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

    return calcTime;
}

uint16_t SCC68070::JSR()
{
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;
    uint32_t pc;

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

    SetLong(ARIWPr(7, 4), PC);
    PC = pc;

    return calcTime;
}

uint16_t SCC68070::LEA()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;

    if(eamode == 2)
    {   A[reg] = A[eareg]; calcTime = 7; }
    else if(eamode == 5)
    {   A[reg] = ARIWD(eareg); calcTime = 14; }
    else if(eamode == 6)
    {   A[reg] = ARIWI8(eareg); calcTime = 17; }
    else
    {
        if(eareg == 0)
        {   A[reg] = ASA(); calcTime = 14; }
        else if(eareg == 1)
        {   A[reg] = ALA(); calcTime = 18; }
        else if(eareg == 2)
        {   A[reg] = PCIWD(); calcTime = 14; }
        else
        {   A[reg] = PCIWI8(); calcTime = 17; }
    }

    return calcTime;
}

uint16_t SCC68070::LINK()
{
    uint8_t reg = (currentOpcode & 0x0007);

    SetLong(ARIWPr(7, 4), A[reg]);
    A[reg] = A[7];

    int16_t disp = GetNextWord();
    A[7] += signExtend16(disp);

    return 25;
}

uint16_t SCC68070::LSm()
{
    uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;
    uint16_t data = GetWord(eamode, eareg, calcTime);

    if(dr) // left
    {
        uint16_t a = data & 0x8000;
        SetXC(a ? 1 : 0);
        data <<= 1;
    }
    else // right
    {
        uint16_t a = data & 0x0001;
        SetXC(a);
        data >>= 1;
    }

    if(data & 0x8000) SetN(); else SetN(0);
    if(data == 0) SetZ(); else SetZ(0);
    SetV(0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::LSr()
{
    uint8_t count = (currentOpcode & 0x0E00) >> 9;
    uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    uint8_t   reg = (currentOpcode & 0x0007);
    uint8_t shift;

    if(ir)
        shift = D[count] % 64;
    else
        shift = (count) ? count : 8;

    if(!shift)
    {
        SetV(0);
        SetC(0);
        if(size == 0)
        {
            uint8_t data = D[reg] & 0x000000FF;
            if(data & 0x80) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
        else if(size == 1)
        {
            uint16_t data = D[reg] & 0x0000FFFF;
            if(data & 0x8000) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
        else
        {
            uint32_t data = D[reg];
            if(data & 0x80000000) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
        return 13;
    }

    if(size == 0) // byte
    {
        uint8_t data = D[reg] & 0x000000FF;
        if(dr)
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x80;
                SetXC(a ? 1 : 0);
                data <<= 1;
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
            for(uint8_t i = 0; i < shift; i++)
            {
                uint16_t a = data & 0x8000;
                SetXC(a ? 1 : 0);
                data <<= 1;
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint16_t a = data & 0x0001;
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
            for(uint8_t i = 0; i < shift; i++)
            {
                uint32_t a = data & 0x80000000;
                SetXC(a ? 1 : 0);
                data <<= 1;
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint32_t a = data & 0x00000001;
                SetXC(a);
                data >>= 1;
            }
        }

        if(data & 0x80000000) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] = data;
    }
    SetV(0);

    return 13 + 3 * shift;
}

uint16_t SCC68070::MOVE()
{
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

    return calcTime;
}

uint16_t SCC68070::MOVEA()
{
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
        A[reg] = GetLong(eamode, eareg, calcTime);
    }

    return calcTime;
}

uint16_t SCC68070::MOVECCR()
{
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 10;

    uint16_t data = GetWord(eamode, eareg, calcTime) & 0x001F;
    SR &= 0xFF00;
    SR |= data;

    return calcTime;
}

uint16_t SCC68070::MOVEfSR() // Should not be used according to the Green Book Chapter VI.2.2.2
{
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    SetWord(eamode, eareg, calcTime, SR);

    return calcTime;
}

uint16_t SCC68070::MOVESR()
{
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 10;

    if(GetS())
    {
        uint16_t data = GetWord(eamode, eareg, calcTime);
        SR = data;
    }
    else
        return Exception(PrivilegeViolation);

    return calcTime;
}

uint16_t SCC68070::MOVEUSP()
{
    uint8_t  dr = (currentOpcode & 0x0008) >> 3;
    uint8_t reg = (currentOpcode & 0x0007);

    if(GetS())
        if(dr)
            A[reg] = USP;
        else
            USP = A[reg];
    else
        return Exception(PrivilegeViolation);

    return 7;
}

uint16_t SCC68070::MOVEM()
{
    uint8_t     dr = (currentOpcode & 0x0400) >> 10;
    uint8_t   size = (currentOpcode & 0x0040) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t  mask = GetNextWord();
    uint16_t calcTime;

    // Prepare the lastAddress member
    if(size)
    {
        GetLong(eamode, eareg, calcTime, NoFlags);
        if(eamode == 3)
            A[eareg] -= 4;
        if(eamode == 4)
        {
            A[eareg] += 4;
            lastAddress += 4;
        }
        size = 4;
    }
    else
    {
        GetWord(eamode, eareg, calcTime, NoFlags);
        if(eamode == 3)
            A[eareg] -= 2;
        if(eamode == 4)
        {
            A[eareg] += 2;
            lastAddress += 2;
        }
        size = 2;
    }

    calcTime = 0;

    uint8_t n = 0;
    int8_t mod;
    bool type = false;
    if(dr) // Memory to register
    {
        mod = 0;
        for(uint8_t i = 0; i < 16; i++)
        {
            if(mask & 1)
            {
                if(size == 4) // long
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
                        D[mod] = signExtend16(GetWord(lastAddress));
                }
                n++;
                lastAddress += size;
            }
            mask >>= 1;
            mod++;
            mod %= 8;
            if(mod == 0)
                type = true; // false means data, true means address
        }
    }
    else // Register to memory
    {
        if(eamode == 4)
            mod = 7;
        else
            mod = 0;
        for(uint8_t i = 0; i < 16; i++)
        {
            if(mask & 1)
            {
                if(eamode == 4)
                    lastAddress -= size;
                if(size == 4) // long
                {
                    if(eamode == 4) // Predecrement addressing uses a different mask
                        if(type) // data
                            SetLong(lastAddress, D[mod]);
                        else // address
                            SetLong(lastAddress, A[mod]);
                    else
                        if(type) // address
                            SetLong(lastAddress, A[mod]);
                        else // data
                            SetLong(lastAddress, D[mod]);
                }
                else // word
                {
                    if(eamode == 4) // Predecrement addressing uses a different mask
                        if(type) // data
                            SetWord(lastAddress, D[mod]);
                        else // address
                            SetWord(lastAddress, A[mod]);
                    else
                        if(type) // address
                            SetWord(lastAddress, A[mod]);
                        else // data
                            SetWord(lastAddress, D[mod]);
                }
                n++;
                if(eamode != 4)
                    lastAddress += size;
            }
            mask >>= 1;
            if(eamode == 4)
                mod--;
            else
                mod++;
            if(mod < 0 || mod > 7)
            {
                type = true; // data or address register
                if(eamode == 4)
                    mod = 7;
                else
                    mod = 0;
            }
        }
    }

    if(eamode == 3 || eamode == 4)
        A[eareg] = lastAddress;

    if(eamode == 2)
        calcTime = 23;
    if(eamode == 3)
        calcTime = 26;
    if(eamode == 4)
        calcTime = 23;
    if(eamode == 5)
        calcTime = 27;
    if(eamode == 6)
        calcTime = 30;
    if(eamode == 7 && eareg == 0)
        calcTime = 27;
    if(eamode == 7 && eareg == 1)
        calcTime = 31;
    if(eamode == 7 && eareg == 2)
        calcTime = 30;
    if(eamode == 7 && eareg == 3)
        calcTime = 33;

    if(dr)
        calcTime += 3;

    return calcTime + n * (size == 4) ? 11 : 7;
}

uint16_t SCC68070::MOVEP()
{
    uint8_t data = (currentOpcode & 0x0E00) >> 9;
    uint8_t dir  = (currentOpcode & 0x0080) >> 7;
    uint8_t size = (currentOpcode & 0x0040) >> 6;
    int16_t disp = GetNextWord();
    uint32_t address = A[currentOpcode & 0x0007] + disp;
    uint16_t calcTime;

    if(dir == 0) // memory to register
    {
        if(size == 0) // word
        {
            D[data] &= 0xFFFF0000;
            D[data] |= GetByte(address) << 8;
            D[data] |= GetByte(address + 2);
            calcTime = 22;
        }
        else // long
        {
            D[data] = 0;
            D[data] |= GetByte(address) << 24;
            D[data] |= GetByte(address + 2) << 16;
            D[data] |= GetByte(address + 4) << 8;
            D[data] |= GetByte(address + 6);
            calcTime = 36;
        }
    }
    else // register to memory
    {
        if(size == 0) // word
        {
            SetByte(address,     (D[data] & 0x0000FF00) >> 8);
            SetByte(address + 2, (D[data] & 0x000000FF));
            calcTime = 25;
        }
        else // long
        {
            SetByte(address,     (D[data] & 0xFF000000) >> 24);
            SetByte(address + 2, (D[data] & 0x00FF0000) >> 16);
            SetByte(address + 4, (D[data] & 0x0000FF00) >> 8);
            SetByte(address + 6, (D[data] & 0x000000FF));
            calcTime = 39;
        }
    }

    return calcTime;
}

uint16_t SCC68070::MOVEQ()
{
    uint8_t  reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t data = (currentOpcode & 0x00FF);

    if(data & 0x80) SetN(); else SetN(0);
    if(data == 0) SetZ(); else SetZ(0);
    SetVC(0);

    D[reg] = signExtend8(data);
    return 7;
}

uint16_t SCC68070::MULS()
{
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

    return calcTime;
}

uint16_t SCC68070::MULU()
{
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

    return calcTime;
}

uint16_t SCC68070::NBCD()
{
    uint8_t mode = (currentOpcode & 0x0038) >> 3;
    uint8_t  reg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;

    uint8_t x = GetX();
    uint8_t data = GetByte(mode, reg, calcTime);
    uint8_t result = 100 - PBCDToByte(data) - x;
    result = byteToPBCD(result);

    SetXC(); // how can there be no borrow here ?

    if(result != 0)
        SetZ(0);

    if(mode == 0)
    {
        D[reg] &= 0xFFFFFF00;
        D[reg] |= result;
        return 10;
    }

    SetByte(lastAddress, result);
    return calcTime;
}

uint16_t SCC68070::NEG()
{
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

    return calcTime;
}

uint16_t SCC68070::NEGX()
{
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

    return calcTime;
}

uint16_t SCC68070::NOP()
{
    return 7;
}

uint16_t SCC68070::NOT()
{
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 0) // Byte
    {
        uint8_t data = GetByte(eamode, eareg, calcTime);
        uint8_t res = ~data;

        if(eamode)
        {   SetByte(lastAddress, res); calcTime += 4; }
        else
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= res; }

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);
    }
    else if(size == 1) // Word
    {
        uint16_t data = GetWord(eamode,  eareg, calcTime);
        uint16_t res = ~data;

        if(eamode)
        {   SetWord(lastAddress, res); calcTime += 4; }
        else
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= res; }

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);
    }
    else // long
    {
        uint32_t data = GetLong(eamode,  eareg, calcTime);
        uint32_t res = ~data;

        if(eamode)
        {   SetLong(lastAddress, res); calcTime += 8; }
        else
            D[eareg] = res;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);
    }

    SetVC(0);

    return calcTime;
}

uint16_t SCC68070::OR()
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
        uint8_t res = src | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

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

        D[reg] = res;
    }
    else if(opmode == 4)
    {
        uint8_t src = D[reg] & 0x000000FF;
        uint8_t dst = GetByte(eamode, eareg, calcTime);
        uint8_t res = src | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        calcTime += 4;
        SetByte(lastAddress, res);
    }
    else if(opmode == 5)
    {
        uint16_t src = D[reg] & 0x0000FFFF;
        uint16_t dst = GetWord(eamode, eareg, calcTime);
        uint16_t res = src | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        calcTime += 4;
        SetWord(lastAddress, res);
    }
    else
    {
        uint32_t src = D[reg];
        uint32_t dst = GetLong(eamode, eareg, calcTime);
        uint32_t res = src | dst;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        calcTime += 8;
        SetLong(lastAddress, res);
    }
    SetVC(0);

    return calcTime;
}

uint16_t SCC68070::ORI()
{
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

    return calcTime;
}

uint16_t SCC68070::ORICCR()
{
    uint8_t data = GetNextWord() & 0x1F;
    SR |= data;
    return 14;
}

uint16_t SCC68070::ORISR()
{
    uint16_t data = GetNextWord();

    if(!GetS())
        return Exception(PrivilegeViolation);

    SR |= data;
    return 14;
}

uint16_t SCC68070::PEA()
{
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime;

    uint32_t pc;
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

    SetLong(ARIWPr(7, 4), pc);

    return calcTime;
}

uint16_t SCC68070::RESET() // Not fully emulated
{
    if(!GetS())
        return Exception(PrivilegeViolation);

    return 154;
}

uint16_t SCC68070::ROm()
{
    uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;
    uint16_t data = GetWord(eamode, eareg, calcTime);

    if(dr) // left
    {
        uint16_t a = data & 0x8000;
        data <<= 1;
        a >>= 15;
        data |= a;
        SetC(a);
    }
    else // right
    {
        uint16_t a = data & 0x0001;
        SetC(a);
        data >>= 1;
        a <<= 15;
        data |= a;
    }

    if(data & 0x8000) SetN(); else SetN(0);
    if(data == 0) SetZ(); else SetZ(0);
    SetV(0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::ROr()
{
    uint8_t count = (currentOpcode & 0x0E00) >> 9;
    uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    uint8_t   reg = (currentOpcode & 0x0007);
    uint8_t shift;

    if(ir)
        shift = D[count] % 64;
    else
        shift = (count) ? count : 8;

    if(!shift)
    {
        SetV(0);
        SetC(0);
        if(size == 0)
        {
            uint8_t data = D[reg] & 0x000000FF;
            if(data & 0x80) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
        else if(size == 1)
        {
            uint16_t data = D[reg] & 0x0000FFFF;
            if(data & 0x8000) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
        else
        {
            uint32_t data = D[reg];
            if(data & 0x80000000) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
        return 13;
    }

    if(size == 0) // byte
    {
        uint8_t data = D[reg] & 0x000000FF;
        if(dr)
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x80;
                data <<= 1;
                a >>= 7;
                data |= a;
                SetC(a);
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x01;
                SetC(a);
                data >>= 1;
                a <<= 7;
                data |= a;
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
            for(uint8_t i = 0; i < shift; i++)
            {
                uint16_t a = data & 0x8000;
                data <<= 1;
                a >>= 15;
                data |= a;
                SetC(a);
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint16_t a = data & 0x0001;
                SetC(a);
                data >>= 1;
                a <<= 15;
                data |= a;
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
            for(uint8_t i = 0; i < shift; i++)
            {
                uint32_t a = data & 0x80000000;
                data <<= 1;
                a >>= 31;
                data |= a;
                SetC(a);
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint32_t a = data & 0x00000001;
                SetC(a);
                data >>= 1;
                a <<= 31;
                data |= a;
            }
        }

        if(data & 0x80000000) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] = data;
    }
    SetV(0);

    return 13 + 3 * shift;
}

uint16_t SCC68070::ROXm()
{
    uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 14;
    uint16_t data = GetWord(eamode, eareg, calcTime);

    if(dr) // left
    {
        uint16_t a = data & 0x8000;
        data <<= 1;
        data |= GetX();
        SetXC(a ? 1 : 0);
    }
    else // right
    {
        uint16_t a = data & 0x0001;
        data >>= 1;
        data |= GetX() << 15;
        SetXC(a);
    }

    if(data & 0x8000) SetN(); else SetN(0);
    if(data == 0) SetZ(); else SetZ(0);
    SetV(0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::ROXr()
{
    uint8_t count = (currentOpcode & 0x0E00) >> 9;
    uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    uint8_t   reg = (currentOpcode & 0x0007);
    uint8_t shift;

    if(ir)
        shift = D[count] % 64;
    else
        shift = (count) ? count : 8;

    if(!shift)
    {
        SetV(0);
        SetC(GetX());
        if(size == 0)
        {
            uint8_t data = D[reg] & 0x000000FF;
            if(data & 0x80) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
        else if(size == 1)
        {
            uint16_t data = D[reg] & 0x0000FFFF;
            if(data & 0x8000) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
        else
        {
            uint32_t data = D[reg];
            if(data & 0x80000000) SetN(); else SetN(0);
            if(data == 0) SetZ(); else SetZ(0);
        }
        return 13;
    }

    if(size == 0) // byte
    {
        uint8_t data = D[reg] & 0x000000FF;
        if(dr)
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x80;
                data <<= 1;
                data |= GetX();
                SetXC(a ? 1 : 0);
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint8_t a = data & 0x01;
                data >>= 1;
                data |= GetX() << 7;
                SetXC(a);
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
            for(uint8_t i = 0; i < shift; i++)
            {
                uint16_t a = data & 0x8000;
                data <<= 1;
                data |= GetX();
                SetXC(a ? 1 : 0);
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint16_t a = data & 0x0001;
                data >>= 1;
                data |= GetX() << 15;
                SetXC(a);
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
            for(uint8_t i = 0; i < shift; i++)
            {
                uint32_t a = data & 0x80000000;
                data <<= 1;
                data |= GetX();
                SetXC(a ? 1 : 0);
            }
        }
        else
        {
            for(uint8_t i = 0; i < shift; i++)
            {
                uint32_t a = data & 0x00000001;
                data >>= 1;
                data |= GetX() << 31;
                SetXC(a);
            }
        }

        if(data & 0x80000000) SetN(); else SetN(0);
        if(data == 0) SetZ(); else SetZ(0);
        D[reg] = data;
    }
    SetV(0);

    return 13 + 3 * shift;
}

uint16_t SCC68070::RTE()
{
    uint16_t calcTime = 0;

    if(GetS())
    {
        calcTime = 39;
        SR = GetWord(ARIWPo(7, 2));
        PC = GetLong(ARIWPo(7, 4));
        uint16_t format = GetWord(ARIWPo(7, 4));

        if(format & 0x0C03)
            calcTime += Exception(FormatError);
        else if(format & 0xF000) // long format
        {
            A[7] += 26;
            calcTime = 146;
        }
    }
    else
        calcTime += Exception(PrivilegeViolation);

    return calcTime;
}

uint16_t SCC68070::RTR()
{
    SR &= 0xFFE0;
    SR |= GetWord(ARIWPo(7, 2)) & 0x001F;
    PC = GetLong(ARIWPo(7, 4));
    return 22;
}

uint16_t SCC68070::RTS()
{
    PC = GetLong(ARIWPo(7, 4));
    return 15;
}

uint16_t SCC68070::SBCD()
{
    uint8_t Ry = (currentOpcode & 0x0E00) >> 9;
    uint8_t Rx = (currentOpcode & 0x0007);
    uint8_t rm = currentOpcode & 0x0008;
    uint16_t calcTime;
    uint8_t result = 0;
    uint8_t x = GetX();

    if(rm) // memory to memory
    {
        uint8_t src = PBCDToByte(GetByte(ARIWPr(Rx, 1))) + x;
        uint8_t dst = PBCDToByte(GetByte(ARIWPr(Ry, 1)));

        if(dst < src)
        {
            dst += 100;
            SetXC();
        }
        else
            SetXC(0);

        result = byteToPBCD(dst - src);

        SetByte(lastAddress, result);
        calcTime = 31;
    }
    else
    {
        uint8_t src = PBCDToByte(D[Rx] & 0x000000FF) + x;
        uint8_t dst = PBCDToByte(D[Ry] & 0x000000FF);

        if(dst < src)
        {
            dst += 100;
            SetXC();
        }
        else
            SetXC(0);

        result = byteToPBCD(dst - src);

        D[Ry] &= 0xFFFFFF00;
        D[Ry] |= result;
        calcTime = 10;
    }

    if(result != 0)
        SetZ(0);

    return calcTime;
}

uint16_t SCC68070::Scc()
{
    uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    uint8_t    eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t     eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = eamode ? 17 : 13;
    uint8_t data;

    if((this->*ConditionalTests[condition])())
        data = 0xFF;
    else
        data = 0x00;

    SetByte(eamode, eareg, calcTime, data);

    return calcTime;
}

uint16_t SCC68070::STOP() // Not fully emulated
{
    uint16_t data = GetNextWord();

    if(GetS())
    {
        loop = false;
        SR = data;
    }
    else
        return Exception(PrivilegeViolation);

    return 13;
}

uint16_t SCC68070::SUB()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 0;

    if(opmode == 0)
    {
        int8_t src = GetByte(eamode, eareg, calcTime);
        int8_t dst = D[reg] & 0x000000FF;
        int16_t res = dst - src;
        uint16_t ures = (uint8_t)dst - (uint8_t)res;

        if(ures & 0x100) SetXC(); else SetXC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] &= 0xFFFFFF00;
        D[reg] |= (uint8_t)ures;
    }
    else if(opmode == 1)
    {
        int16_t src = GetWord(eamode, eareg, calcTime);
        int16_t dst = D[reg] & 0x0000FFFF;
        int32_t res = dst - src;
        uint32_t ures = (uint16_t)dst - (uint16_t)res;

        if(ures & 0x10000) SetXC(); else SetXC(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] &= 0xFFFF0000;
        D[reg] |= (uint16_t)ures;
    }
    else if(opmode == 2)
    {
        int32_t src = GetLong(eamode, eareg, calcTime);
        int32_t dst = D[reg];
        int64_t res = dst - src;
        uint64_t ures = (uint32_t)dst - (uint32_t)res;

        if(ures & 0x100000000) SetXC(); else SetXC(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] = (uint32_t)ures;
    }
    else if(opmode == 4)
    {
        int8_t src = D[reg] & 0x000000FF;
        int8_t dst = GetByte(eamode, eareg, calcTime);
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
        int16_t src = D[reg] & 0x0000FFFF;
        int16_t dst = GetWord(eamode, eareg, calcTime);
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
        int32_t src = D[reg];
        int32_t dst = GetLong(eamode, eareg, calcTime);
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

uint16_t SCC68070::SUBA()
{
    uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    uint8_t opmode = (currentOpcode & 0x0100) >> 8;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;
    int32_t src;

    if(opmode) // Long
    {
        src = GetLong(eamode, eareg, calcTime);
        A[reg] -= src;
        if(eamode > 1)
            calcTime += 8;
    }
    else // Word
    {
        src = signExtend16(GetWord(eamode, eareg, calcTime));
        int16_t dst = A[reg] & 0x0000FFFF;
        A[reg] = dst - src;
        if(eamode > 1)
            calcTime += 4;
    }


    return calcTime;
}

uint16_t SCC68070::SUBI()
{
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
        {   D[eareg] = (uint32_t)res; calcTime = 18; }
    }

    return calcTime;
}

uint16_t SCC68070::SUBQ()
{
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
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= (uint8_t)ures; }
        else
        {   SetByte(lastAddress, ures); calcTime += 4; }
    }
    else if(size == 1) // word
    {
        int16_t dst = GetWord(eamode, eareg, calcTime);
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
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= (uint16_t)ures; }
        else if(eamode == 1)
            A[eareg] = signExtend16(res);
        else
        {   SetWord(lastAddress, ures); calcTime += 4; }
    }
    else // long
    {
        int32_t dst = GetLong(eamode, eareg, calcTime);
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
            D[eareg] = ures;
        else if(eamode == 1)
            A[eareg] = ures;
        else
        {   SetLong(lastAddress, ures); calcTime += 8; }
    }

    return calcTime;
}

uint16_t SCC68070::SUBX()
{
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
            D[ry] |= (uint8_t)ures;
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
            D[ry] |= (uint16_t)ures;
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

            D[ry] = (uint32_t)ures;
            calcTime = 7;
        }
    }

    return calcTime;
}

uint16_t SCC68070::SWAP()
{
    uint8_t reg = currentOpcode & 0x0007;

    uint16_t tmp = (D[reg] & 0xFFFF0000) >> 16;
    D[reg] <<= 16;
    D[reg] |= tmp;

    if(D[reg] == 0) SetZ(); else SetZ(0);
    if(D[reg] & 0x80000000) SetN(); else SetN(0);

    SetVC(0);

    return 7;
}

uint16_t SCC68070::TAS()
{
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 10;

    uint8_t data = GetByte(eamode, eareg, calcTime);

    if(data == 0) SetZ(); else SetZ(0);
    if(data & 0x80) SetN(); else SetN(0);
    data |= 0x80;

    SetVC(0);

    if(eamode)
    {   calcTime++; SetByte(lastAddress, data); }
    else
    {   D[eareg] &= 0xFFFFFF00; D[eareg] |= data; }

    return calcTime;
}

uint16_t SCC68070::TRAP()
{
    uint8_t vec = currentOpcode & 0x000F;
    uint16_t calcTime = 0;

    calcTime += Exception(32 + vec);

    return calcTime;
}

uint16_t SCC68070::TRAPV()
{
    uint16_t calcTime = 0;

    if(GetV())
        calcTime += Exception(TRAPVInstruction);
    else
        calcTime = 10;

    return calcTime;
}

uint16_t SCC68070::TST()
{
    uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    uint8_t  eareg = (currentOpcode & 0x0007);
    uint16_t calcTime = 7;

    if(size == 0)
    {
        uint8_t data = GetByte(eamode, eareg, calcTime);

        if(!data) SetZ(); else SetZ(0);
        if(data & 0x80) SetN(); else SetN(0);
    }
    else if(size == 1)
    {
        uint16_t data = GetWord(eamode, eareg, calcTime);

        if(!data) SetZ(); else SetZ(0);
        if(data & 0x8000) SetN(); else SetN(0);
    }
    else
    {
        uint32_t data = GetLong(eamode, eareg, calcTime);

        if(!data) SetZ(); else SetZ(0);
        if(data & 0x80000000) SetN(); else SetN(0);
    }

    SetVC(0);

    return calcTime;
}

uint16_t SCC68070::UNLK()
{
    uint8_t reg = currentOpcode & 0x0007;

    A[7] = A[reg];
    A[reg] = GetLong(ARIWPo(7, 4));

    return 15;
}
