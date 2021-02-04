#include "SCC68070.hpp"
#include "../../boards/Board.hpp"
#include "../../common/utils.hpp"

bool operator>(const SCC68070Exception& lhs, const SCC68070Exception& rhs)
{
    return lhs.group > rhs.group;
}

uint16_t SCC68070::Exception(const uint8_t vectorNumber)
{
    uint16_t calcTime = 0;
    uint16_t sr = SR;
    SetS();

    if(vectorNumber == ResetSSPPC)
    {
        SSP = board->GetLong(0, Trigger);
        A[7] = SSP;
        PC = board->GetLong(4, Trigger);
        SR = 0x2700;
        USP = 0;
		return 0;
    }

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
    case CHKInstruction:
        calcTime += 64; break;
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
    exceptions.push({IllegalInstruction, 1});
    return 0;
}

uint16_t SCC68070::ABCD()
{
    const uint8_t rx = currentOpcode >> 9 & 0x0007;
    const bool rm = currentOpcode >> 3 & 0x0001;
    const uint8_t ry = currentOpcode & 0x0007;
    uint16_t calcTime;

    uint8_t dst, src;
    if(rm) // Memory to Memory
    {
        src = PBCDToByte(GetByte(ARIWPr(ry, 1)));
        dst = PBCDToByte(GetByte(ARIWPr(rx, 1)));
        calcTime = 31;
    }
    else // Data register to Data register
    {
        src = PBCDToByte(D[ry] & 0xFF);
        dst = PBCDToByte(D[rx] & 0xFF);
        calcTime = 10;
    }

    const uint8_t result = src + dst + GetX();

    if(result != 0)
        SetZ(0);
    SetXC(result > 99);

    if(rm)
        SetByte(lastAddress, byteToPBCD(result));
    else
    {
        D[rx] &= 0xFFFFFF00;
        D[rx] |= byteToPBCD(result);
    }

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
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t opmode = currentOpcode >> 8 & 0x0001;
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    uint32_t src, dst, dataMask, msb;
    if(size == 0) // Byte
    {
        dataMask = 0x000000FF;
        msb = 1 << 7;
        src = opmode ? (D[reg] & dataMask) : GetByte(eamode, eareg, calcTime);
        dst = opmode ? GetByte(eamode, eareg, calcTime) : (D[reg] & dataMask);
    }
    else if(size == 1) // Word
    {
        dataMask = 0x0000FFFF;
        msb = 1 << 15;
        src = opmode ? (D[reg] & dataMask) : GetWord(eamode, eareg, calcTime);
        dst = opmode ? GetWord(eamode, eareg, calcTime) : (D[reg] & dataMask);
    }
    else // Long
    {
        dataMask = 0xFFFFFFFF;
        msb = 1 << 31;
        src = opmode ? D[reg] : GetLong(eamode, eareg, calcTime);
        dst = opmode ? GetLong(eamode, eareg, calcTime) : D[reg];
    }

    dst &= src;

    SetN(dst & msb);
    SetZ((dst & dataMask) == 0);
    SetVC(0);

    if(opmode) // Memory
    {
        if(size == 0) // Byte
            SetByte(lastAddress, dst & dataMask);
        else if(size == 1) // Word
            SetWord(lastAddress, dst & dataMask);
        else // Long
            SetLong(lastAddress, dst);
        calcTime += (size == 2) ? 8 : 4;
    }
    else // Register
    {
        D[reg] &= ~dataMask;
        D[reg] |= dst & dataMask;
    }

    return calcTime;
}

uint16_t SCC68070::ANDI()
{
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 0;

    uint32_t data, dst, dstMask, msb;
    if(size == 0) // Byte
    {
        data = GetNextWord() & 0x00FF;
        dst = GetByte(eamode, eareg, calcTime);
        dstMask = 0x000000FF;
        msb = 1 << 7;
    }
    else if(size == 1) // Word
    {
        data = GetNextWord();
        dst = GetWord(eamode, eareg, calcTime);
        dstMask = 0x0000FFFF;
        msb = 1 << 15;
    }
    else // Long
    {
        data = GetNextWord() << 16 | GetNextWord();
        dst = GetLong(eamode, eareg, calcTime);
        dstMask = 0xFFFFFFFF;
        msb = 1 << 31;
        calcTime += eamode ? 8 : 4;
    }

    dst &= data;

    SetN(dst & msb);
    SetZ((dst & dstMask) == 0);
    SetVC(0);

    if(eamode)
    {
        if(size == 0) // Byte
            SetByte(lastAddress, dst & dstMask);
        else if(size == 1) // Word
            SetWord(lastAddress, dst & dstMask);
        else // Long
            SetLong(lastAddress, dst);
        calcTime += 18;
    }
    else
    {
        D[eareg] &= ~dstMask;
        D[eareg] |= dst & dstMask;
        calcTime += 14;
    }

    return calcTime;
}

uint16_t SCC68070::ANDICCR()
{
    const uint16_t data = 0xA700 | (GetNextWord() & 0x001F);
    SR &= data;
    return 14;
}

uint16_t SCC68070::ANDISR()
{
    const uint16_t data = GetNextWord();

    if(!GetS())
    {
        exceptions.push({PrivilegeViolation, 1});
        return 0;
    }

    SR &= data;
    if(!GetS()) // S bit changed from supervisor to user
    {
        SSP = A[7];
        A[7] = USP;
    }
    SR &= 0xA71F;

    return 14;
}

uint16_t SCC68070::ASm()
{
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 14;

    uint16_t data = GetWord(eamode, eareg, calcTime);
    if(currentOpcode & 0x0100) // Left
    {
        const uint16_t sign = data & 0x8000;
        SetXC(sign);
        data <<= 1;
        SetV(sign ^ (data & 0x8000));
    }
    else // Right
    {
        // TODO: use C++20 behaviour when right shifting signed data (int16_t and without sign).
        const uint16_t sign = data & 0x8000;
        SetXC(data & 1);
        data >>= 1;
        data |= sign;
        SetV(0);
    }

    SetN(data & 0x8000);
    SetZ(data == 0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::ASr()
{
    const uint8_t count = currentOpcode >> 9 & 0x0007;
    const uint8_t  size = currentOpcode >> 6 & 0x0003;
    const uint8_t   reg = currentOpcode & 0x0007;
    const uint8_t shift = currentOpcode & 0x0020 ? D[count] % 64 : (count ? count : 8);

    SetVC(0);

    uint32_t data, dataMask, signMask;
    if(size == 0) // Byte
    {
        dataMask = 0x000000FF;
        signMask = 0x00000080;
    }
    else if(size == 1) // Word
    {
        dataMask = 0x0000FFFF;
        signMask = 0x00008000;
    }
    else // Long
    {
        dataMask = 0xFFFFFFFF;
        signMask = 0x80000000;
    }
    data = D[reg] & dataMask;

    if(currentOpcode & 0x0100) // Left
    {
        for(uint8_t i = shift; i > 0; i--)
        {
            const uint32_t sign = data & signMask;
            SetXC(sign);
            data <<= 1;
            if(sign ^ (data & signMask))
                SetV();
        }
    }
    else // Right
    {
        const uint32_t sign = data & signMask;
        for(uint8_t i = shift; i > 0; i--)
        {
            SetXC(data & 1);
            data >>= 1;
            data |= sign;
        }
    }

    SetN(data & signMask);
    SetZ((data & dataMask) == 0);

    D[reg] &= ~dataMask;
    D[reg] |= data & dataMask;

    return 13 + 3 * shift;
}

uint16_t SCC68070::Bcc()
{
    const uint8_t condition = currentOpcode >> 8 & 0x000F;
    int16_t disp = signExtend<int8_t, int16_t>(currentOpcode & 0x00FF);
    const uint32_t pc = PC;
    uint16_t calcTime = 13;

    if(disp == 0)
    {
        disp = GetNextWord();
        calcTime++;
    }

    if((this->*ConditionalTests[condition])())
        PC = pc + disp;

    return calcTime;
}

uint16_t SCC68070::BCHG()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 10;

    uint8_t bit;
    if(currentOpcode & 0x0100) // Dynamic
    {
        bit = D[reg];
    }
    else // Static
    {
        bit = GetNextWord() & 0x00FF;
        calcTime += 7;
    }

    if(eamode) // Byte
    {
        bit %= 8;
        const uint8_t mask = 1 << bit;
        uint8_t data = GetByte(eamode, eareg, calcTime);
        SetZ(!(data & mask));
        data ^= mask;
        SetByte(lastAddress, data);
        calcTime += 4;
    }
    else // Long
    {
        bit %= 32;
        const uint32_t mask = 1 << bit;
        SetZ(!(D[eareg] & mask));
        D[eareg] ^= mask;
    }

    return calcTime;
}

uint16_t SCC68070::BCLR()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 10;

    uint8_t bit;
    if(currentOpcode & 0x0100) // Dynamic
    {
        bit = D[reg];
    }
    else // Static
    {
        bit = GetNextWord() & 0x00FF;
        calcTime += 7;
    }

    if(eamode) // Byte
    {
        bit %= 8;
        const uint8_t mask = 1 << bit;
        uint8_t data = GetByte(eamode, eareg, calcTime);
        SetZ(!(data & mask));
        data &= ~mask;
        SetByte(lastAddress, data);
        calcTime += 4;
    }
    else // Long
    {
        bit %= 32;
        const uint32_t mask = 1 << bit;
        SetZ(!(D[eareg] & mask));
        D[eareg] &= ~mask;
    }

    return calcTime;
}

uint16_t SCC68070::BRA()
{
    int16_t disp = signExtend<int8_t, int16_t>(currentOpcode & 0x00FF);
    const uint32_t pc = PC;
    uint16_t calcTime = 13;

    if(disp == 0)
    {
        disp = GetNextWord();
        calcTime++;
    }

    PC = pc + disp;

    return calcTime;
}

uint16_t SCC68070::BSET()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 10;

    uint8_t bit;
    if(currentOpcode & 0x0100) // Dynamic
    {
        bit = D[reg];
    }
    else // Static
    {
        bit = GetNextWord() & 0x01FF; // Error in the datasheet ?
        calcTime += 7;
    }

    if(eamode) // Byte
    {
        bit %= 8;
        const uint8_t mask = 1 << bit;
        uint8_t data = GetByte(eamode, eareg, calcTime);
        SetZ(!(data & mask));
        data |= mask;
        SetByte(lastAddress, data);
        calcTime += 4;
    }
    else // Long
    {
        bit %= 32;
        const uint32_t mask = 1 << bit;
        SetZ(!(D[eareg] & mask));
        D[eareg] |= mask;
    }

    return calcTime;
}

uint16_t SCC68070::BSR()
{
    int16_t disp = signExtend<int8_t, int16_t>(currentOpcode & 0x00FF);
    uint32_t pc = PC;
    uint16_t calcTime = 17;

    if(disp == 0)
    {
        disp = GetNextWord();
        calcTime = 22;
    }

    SetLong(ARIWPr(7, 4), PC);
    PC = pc + disp;

    return calcTime;
}

uint16_t SCC68070::BTST()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    uint8_t bit;
    if(currentOpcode & 0x0100) // Dynamic
    {
        bit = D[reg];
    }
    else // Static
    {
        bit = GetNextWord() & 0x00FF;
        calcTime += 7;
    }

    if(eamode) // Byte
    {
        bit %= 8;
        const uint8_t mask = 1 << bit;
        const uint8_t data = GetByte(eamode, eareg, calcTime);
        SetZ(!(data & mask));
    }
    else // Long
    {
        bit %= 32;
        const uint32_t mask = 1 << bit;
        SetZ(!(D[eareg] & mask));
    }

    return calcTime;
}

uint16_t SCC68070::CHK()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 0;

    const int16_t bound = GetWord(eamode, eareg, calcTime);
    const int16_t  data = D[reg] & 0x0000FFFF;
    if(data < 0 || data > bound)
    {
        exceptions.push({CHKInstruction, 2});
        if(data < 0)
            SetN();
        else if(data > bound)
            SetN(0);
    }
    else
        calcTime += 19;

    return calcTime;
}

uint16_t SCC68070::CLR()
{
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 0) // Byte
        SetByte(eamode, eareg, calcTime, 0); // Subtract one read cycle from effective address calculation
    else if(size == 1) // Word
        SetWord(eamode, eareg, calcTime, 0); // Subtract one read cycle from effective address calculation
    else // Long
        SetLong(eamode, eareg, calcTime, 0); // Subtract two read cycles from effective address calculation

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

        if((uint8_t)src > (uint8_t)dst) SetC(); else SetC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);
    }
    else if(opmode == 1) // Word
    {
        int16_t src = GetWord(eamode, eareg, calcTime);
        int16_t dst = D[reg] &= 0x0000FFFF;
        int32_t res = dst - src;

        if((uint16_t)src > (uint16_t)dst) SetC(); else SetC(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);
    }
    else // Long
    {
        int32_t src = GetLong(eamode, eareg, calcTime);
        int32_t dst = D[reg];
        int64_t res = dst - src;

        if((uint32_t)src > (uint32_t)dst) SetC(); else SetC(0);
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

    if(opmode) // Long
    {
        src = GetLong(eamode, eareg, calcTime);
        dst = A[reg];
        res = dst - src;
    }
    else // Word
    {
        src = signExtend<int16_t, int32_t>(GetWord(eamode, eareg, calcTime));
        dst = signExtend<int16_t, int32_t>(A[reg] &= 0x0000FFFF);
        res = dst - src;
    }

    if((uint32_t)src > (uint32_t)dst) SetC(); else SetC(0);
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

        if((uint8_t)data > (uint8_t)dst) SetC(); else SetC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);
    }
    else if(size == 1) // Word
    {
        int16_t data = GetNextWord();
        int16_t dst = GetWord(eamode, eareg, calcTime);
        int32_t res = dst - data;

        if((uint16_t)data > (uint16_t)dst) SetC(); else SetC(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);
    }
    else // Long
    {
        int32_t data = (GetNextWord() << 16) | GetNextWord();
        int32_t dst = GetLong(eamode, eareg, calcTime);
        int64_t res = dst - data;

        if((uint32_t)data > (uint32_t)dst) SetC(); else SetC(0);
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
        if(res & 0x80) SetN(); else SetN(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if((uint8_t)src > (uint8_t)dst) SetC(); else SetC(0);
    }
    else if(size == 1) // word
    {
        int16_t src = GetWord(ARIWPo(Ay, 2));
        int16_t dst = GetWord(ARIWPo(Ax, 2));
        int32_t res = dst - src;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if((uint16_t)src > (uint16_t)dst) SetC(); else SetC(0);
    }
    else // long
    {
        int32_t src = GetLong(ARIWPo(Ay, 4));
        int32_t dst = GetLong(ARIWPo(Ax, 4));
        int64_t res = dst - src;

        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if((uint32_t)src > (uint32_t)dst) SetC(); else SetC(0);
    }

    return (size == 2) ? 26 : 18;
}

uint16_t SCC68070::DBcc()
{
    const uint8_t condition = currentOpcode >> 8 & 0x000F;
    const uint8_t       reg = currentOpcode & 0x0007;
    const int16_t disp = GetNextWord();

    if((this->*ConditionalTests[condition])())
        return 14;

    int16_t counter = D[reg] & 0x0000FFFF;
    counter--;
    D[reg] &= 0xFFFF0000;
    D[reg] |= (uint16_t)counter;
    if(counter == -1)
        return 17;

    PC += disp - 2;

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
    {
        exceptions.push({ZeroDivide, 2});
        return 7; // arbitrary
    }

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
    {
        exceptions.push({ZeroDivide, 2});
        return 7; // arbitrary
    }

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
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    uint32_t src, dst, dataMask, msb;
    if(size == 0) // Byte
    {
        dataMask = 0x000000FF;
        msb = 1 << 7;
        src = D[reg] & dataMask;
        dst = GetByte(eamode, eareg, calcTime);
    }
    else if(size == 1) // Word
    {
        dataMask = 0x0000FFFF;
        msb = 1 << 15;
        src = D[reg] & dataMask;
        dst = GetWord(eamode, eareg, calcTime);
    }
    else // Long
    {
        dataMask = 0xFFFFFFFF;
        msb = 1 << 31;
        src = D[reg];
        dst = GetLong(eamode, eareg, calcTime);
    }

    dst ^= src;

    SetN(dst & msb);
    SetZ((dst & dataMask) == 0);
    SetVC(0);

    if(eamode)
    {
        if(size == 0) // Byte
            SetByte(lastAddress, dst & dataMask);
        else if(size == 1) // Word
            SetWord(lastAddress, dst & dataMask);
        else // Long
            SetLong(lastAddress, dst);
        calcTime += (size == 2) ? 8 : 4;
    }
    else
    {
        D[eareg] &= ~dataMask;
        D[eareg] |= dst & dataMask;
    }

    return calcTime;
}

uint16_t SCC68070::EORI()
{
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 0;

    uint32_t data, dst, dstMask, msb;
    if(size == 0) // Byte
    {
        data = GetNextWord() & 0x00FF;
        dst = GetByte(eamode, eareg, calcTime);
        dstMask = 0x000000FF;
        msb = 1 << 7;
    }
    else if(size == 1) // Word
    {
        data = GetNextWord();
        dst = GetWord(eamode, eareg, calcTime);
        dstMask = 0x0000FFFF;
        msb = 1 << 15;
    }
    else // Long
    {
        data = GetNextWord() << 16 | GetNextWord();
        dst = GetLong(eamode, eareg, calcTime);
        dstMask = 0xFFFFFFFF;
        msb = 1 << 31;
        calcTime += eamode ? 8 : 4;
    }

    dst ^= data;

    SetN(dst & msb);
    SetZ((dst & dstMask) == 0);
    SetVC(0);

    if(eamode)
    {
        if(size == 0) // Byte
            SetByte(lastAddress, dst & dstMask);
        else if(size == 1) // Word
            SetWord(lastAddress, dst & dstMask);
        else // Long
            SetLong(lastAddress, dst);
        calcTime += 18;
    }
    else
    {
        D[eareg] &= ~dstMask;
        D[eareg] |= dst & dstMask;
        calcTime += 14;
    }

    return calcTime;
}

uint16_t SCC68070::EORICCR()
{
    const uint16_t data = GetNextWord() & 0x001F;
    SR ^= data;
    SR &= 0xA71F;
    return 14;
}

uint16_t SCC68070::EORISR()
{
    const uint16_t data = GetNextWord();

    if(!GetS())
    {
        exceptions.push({PrivilegeViolation, 1});
        return 0;
    }

    SR ^= data;
    if(!GetS()) // S bit changed from supervisor to user
    {
        SSP = A[7];
        A[7] = USP;
    }
    SR &= 0xA71F; // Set all unimplemented bytes to 0.

    return 14;
}

uint16_t SCC68070::EXG()
{
    const uint8_t     rx = currentOpcode >> 9 & 0x0007;
    const uint8_t opmode = currentOpcode >> 3 & 0x001F;
    const uint8_t     ry = currentOpcode & 0x0007;

    if(opmode == 0b01000) // Data registers
    {
        const uint32_t tmp = D[rx];
        D[rx] = D[ry];
        D[ry] = tmp;
    }
    else if(opmode == 0b01001) // Address registers
    {
        const uint32_t tmp = A[rx];
        A[rx] = A[ry];
        A[ry] = tmp;
    }
    else // Data register and address register
    {
        const uint32_t tmp = D[rx];
        D[rx] = A[ry];
        A[ry] = tmp;
    }

    return 13;
}

uint16_t SCC68070::EXT()
{
    const uint8_t opmode = currentOpcode >> 6 & 0x0007;
    const uint8_t    reg = currentOpcode & 0x0007;

    if(opmode == 2) // byte to word
    {
        D[reg] = (D[reg] & 0xFFFF0000) | signExtend<int8_t, uint16_t>(D[reg] & 0x000000FF);
        SetN(D[reg] & 0x00008000);
        SetZ((D[reg] & 0x0000FFFF) == 0);
    }
    else // word to long
    {
        D[reg] = signExtend<int16_t, int32_t>(D[reg] & 0x0000FFFF);
        SetN(D[reg] & 0x80000000);
        SetZ(D[reg] == 0);
    }

    SetVC(0);

    return 7;
}

uint16_t SCC68070::ILLEGAL()
{
    exceptions.push({IllegalInstruction, 1});
    return 0; // TODO
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
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = (eamode == 7 && eareg <= 1) ? 6 : 3;

    A[reg] = GetEffectiveAddress(eamode, eareg, 2, calcTime);
    // 2 so it uses the byte/word addressing timing, which is better for calculation time.

    return calcTime;
}

uint16_t SCC68070::LINK()
{
    const uint8_t reg = currentOpcode & 0x0007;

    SetLong(ARIWPr(7, 4), A[reg]);
    A[reg] = A[7];
    A[7] += signExtend<int16_t, int32_t>(GetNextWord());

    return 25;
}

uint16_t SCC68070::LSm()
{
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 14;

    uint16_t data = GetWord(eamode, eareg, calcTime);
    if(currentOpcode & 0x0100) // Left
    {
        SetXC(data & 0x8000);
        data <<= 1;
    }
    else // Right
    {
        SetXC(data & 1);
        data >>= 1;
    }

    SetN(data & 0x8000);
    SetZ(data == 0);
    SetV(0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::LSr()
{
    const uint8_t count = currentOpcode >> 9 & 0x0007;
    const uint8_t  size = currentOpcode >> 6 & 0x0003;
    const uint8_t   reg = currentOpcode & 0x0007;
    const uint8_t shift = currentOpcode & 0x0020 ? D[count] % 64 : (count ? count : 8);

    SetVC(0);

    uint32_t data, dataMask, msbMask;
    if(size == 0) // Byte
    {
        dataMask = 0x000000FF;
        msbMask  = 0x00000080;
    }
    else if(size == 1) // Word
    {
        dataMask = 0x0000FFFF;
        msbMask  = 0x00008000;
    }
    else // Long
    {
        dataMask = 0xFFFFFFFF;
        msbMask  = 0x80000000;
    }
    data = D[reg] & dataMask;

    if(currentOpcode & 0x0100) // Left
    {
        for(uint8_t i = shift; i > 0; i--)
        {
            SetXC(data & msbMask);
            data <<= 1;
        }
    }
    else // Right
    {
        for(uint8_t i = shift; i > 0; i--)
        {
            SetXC(data & 1);
            data >>= 1;
        }
    }

    SetN(data & msbMask);
    SetZ((data & dataMask) == 0);

    D[reg] &= ~dataMask;
    D[reg] |= data & dataMask;

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
        A[reg] = signExtend<int16_t, int32_t>(data);
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
    {
        exceptions.push({PrivilegeViolation, 1});
        return 0;
    }

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
    {
        exceptions.push({PrivilegeViolation, 1});
        return 0;
    }

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
                        A[mod] = signExtend<int16_t, int32_t>(GetWord(lastAddress));
                    else // data
                        D[mod] = signExtend<int16_t, int32_t>(GetWord(lastAddress));
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

    D[reg] = signExtend<int8_t, int32_t>(data);
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
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = eamode ? 14 : 10;

    const uint8_t dst = PBCDToByte(GetByte(eamode, eareg, calcTime));
    int8_t result = 0 - dst - GetX();

    if(result != 0)
        SetZ(0);
    SetXC(result < 0);
    result = 100 + result;

    if(eamode)
        SetByte(lastAddress, byteToPBCD(result));
    else
    {
        D[eareg] &= 0xFFFFFF00;
        D[eareg] |= byteToPBCD(result);
    }

    return calcTime;
}

uint16_t SCC68070::NEG()
{
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 0) // Byte
    {
        const uint8_t data = -(int8_t)GetByte(eamode, eareg, calcTime);
        SetN(data & 0x80);
        SetZ(data == 0);
        SetV(data == 0x80); // negate of -128 is 128, which overflows back to -128.
        SetXC(data != 0);

        if(eamode)
        {
            SetByte(lastAddress, data);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFFFF00;
            D[eareg] |= data;
        }
    }
    else if(size == 1) // Word
    {
        const uint16_t data = -(int16_t)GetWord(eamode, eareg, calcTime);
        SetN(data & 0x8000);
        SetZ(data == 0);
        SetV(data == 0x8000);
        SetXC(data != 0);

        if(eamode)
        {
            SetWord(lastAddress, data);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFF0000;
            D[eareg] |= data;
        }
    }
    else // Long
    {
        const uint32_t data = -(int32_t)GetLong(eamode, eareg, calcTime);
        SetN(data & 0x80000000);
        SetZ(data == 0);
        SetV(data == 0x80000000);
        SetXC(data != 0);

        if(eamode)
        {
            SetLong(lastAddress, data);
            calcTime += 8;
        }
        else
            D[eareg] = data;
    }

    return calcTime;
}

uint16_t SCC68070::NEGX()
{
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 0) // Byte
    {
        const uint8_t data = -(int8_t)GetByte(eamode, eareg, calcTime) - GetX();
        SetN(data & 0x80);
        if(data != 0)
            SetZ(0);
        SetV(data == 0x80);
        SetXC(data != 0);

        if(eamode)
        {
            SetByte(lastAddress, data);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFFFF00;
            D[eareg] |= data;
        }
    }
    else if(size == 1) // Word
    {
        const uint16_t data = -(int16_t)GetWord(eamode, eareg, calcTime) - GetX();
        SetN(data & 0x8000);
        if(data != 0)
            SetZ(0);
        SetV(data == 0x8000);
        SetXC(data != 0);

        if(eamode)
        {
            SetWord(lastAddress, data);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFF0000;
            D[eareg] |= data;
        }
    }
    else // Long
    {
        const uint32_t data = -(int32_t)GetLong(eamode, eareg, calcTime) - GetX();
        SetN(data & 0x80000000);
        if(data != 0)
            SetZ(0);
        SetV(data == 0x80000000);
        SetXC(data != 0);

        if(eamode)
        {
            SetLong(lastAddress, data);
            calcTime += 8;
        }
        else
            D[eareg] = data;
    }

    return calcTime;
}

uint16_t SCC68070::NOP()
{
    return 7;
}

uint16_t SCC68070::NOT()
{
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 0) // Byte
    {
        const uint8_t data = ~GetByte(eamode, eareg, calcTime);
        SetN(data & 0x80);
        SetZ(data == 0);

        if(eamode)
        {
            SetByte(lastAddress, data);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFFFF00;
            D[eareg] |= data;
        }
    }
    else if(size == 1) // Word
    {
        const uint16_t data = ~GetWord(eamode, eareg, calcTime);
        SetN(data & 0x8000);
        SetZ(data == 0);

        if(eamode)
        {
            SetWord(lastAddress, data);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFF0000;
            D[eareg] |= data;
        }
    }
    else // Long
    {
        const uint32_t data = ~GetLong(eamode, eareg, calcTime);
        SetN(data & 0x80000000);
        SetZ(data == 0);

        if(eamode)
        {
            SetLong(lastAddress, data);
            calcTime += 8;
        }
        else
            D[eareg] = data;
    }

    SetVC(0);

    return calcTime;
}

uint16_t SCC68070::OR()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t opmode = currentOpcode >> 8 & 0x0001;
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    uint32_t src, dst, dataMask, msb;
    if(size == 0) // Byte
    {
        dataMask = 0x000000FF;
        msb = 1 << 7;
        src = opmode ? (D[reg] & dataMask) : GetByte(eamode, eareg, calcTime);
        dst = opmode ? GetByte(eamode, eareg, calcTime) : (D[reg] & dataMask);
    }
    else if(size == 1) // Word
    {
        dataMask = 0x0000FFFF;
        msb = 1 << 15;
        src = opmode ? (D[reg] & dataMask) : GetWord(eamode, eareg, calcTime);
        dst = opmode ? GetWord(eamode, eareg, calcTime) : (D[reg] & dataMask);
    }
    else // Long
    {
        dataMask = 0xFFFFFFFF;
        msb = 1 << 31;
        src = opmode ? D[reg] : GetLong(eamode, eareg, calcTime);
        dst = opmode ? GetLong(eamode, eareg, calcTime) : D[reg];
    }

    dst |= src;

    SetN(dst & msb);
    SetZ((dst & dataMask) == 0);
    SetVC(0);

    if(opmode)
    {
        if(size == 0) // Byte
            SetByte(lastAddress, dst & dataMask);
        else if(size == 1) // Word
            SetWord(lastAddress, dst & dataMask);
        else // Long
            SetLong(lastAddress, dst);
        calcTime += size == 2 ? 8 : 4;
    }
    else
    {
        D[reg] &= ~dataMask;
        D[reg] |= dst & dataMask;
    }

    return calcTime;
}

uint16_t SCC68070::ORI()
{
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 14;

    uint32_t data, dst, dstMask, msb;
    if(size == 0) // Byte
    {
        data = GetNextWord() & 0x00FF;
        dst = GetByte(eamode, eareg, calcTime);
        dstMask = 0x000000FF;
        msb = 1 << 7;
    }
    else if(size == 1) // Word
    {
        data = GetNextWord();
        dst = GetWord(eamode, eareg, calcTime);
        dstMask = 0x0000FFFF;
        msb = 1 << 15;
    }
    else // Long
    {
        data = GetNextWord() << 16 | GetNextWord();
        dst = GetLong(eamode, eareg, calcTime);
        dstMask = 0xFFFFFFFF;
        msb = 1 << 31;
        calcTime += eamode ? 8 : 4;
    }

    dst |= data;

    SetN(dst & msb);
    SetZ((dst & dstMask) == 0);
    SetVC(0);

    if(eamode)
    {
        if(size == 0) // Byte
            SetByte(lastAddress, dst & dstMask);
        else if(size == 1) // Word
            SetWord(lastAddress, dst & dstMask);
        else // Long
            SetLong(lastAddress, dst);
        calcTime += 4;
    }
    else
    {
        D[eareg] &= ~dstMask;
        D[eareg] |= dst & dstMask;
    }

    return calcTime;
}

uint16_t SCC68070::ORICCR()
{
    const uint16_t data = GetNextWord() & 0x001F;
    SR |= data;
    SR &= 0xA71F;
    return 14;
}

uint16_t SCC68070::ORISR()
{
    const uint16_t data = GetNextWord();

    if(!GetS())
    {
        exceptions.push({PrivilegeViolation, 1});
        return 0;
    }

    SR |= data;
    // S bit can't change here because it is already 1.
    SR &= 0xA71F;

    return 14;
}

uint16_t SCC68070::PEA()
{
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = (eamode == 7 && eareg <= 1) ? 13 : 10;

    const uint32_t addr = GetEffectiveAddress(eamode, eareg, 4, calcTime);
    SetLong(ARIWPr(7, 4), addr);

    return calcTime;
}

uint16_t SCC68070::RESET()
{
    if(!GetS())
    {
        exceptions.push({PrivilegeViolation, 1});
        return 0;
    }

    board->Reset(false);
    return 154;
}

uint16_t SCC68070::ROm()
{
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 14;

    uint16_t data = GetWord(eamode, eareg, calcTime);
    if(currentOpcode & 0x0100) // Left
    {
        const uint16_t msb = data >> 15;
        SetC(msb);
        data <<= 1;
        data |= msb;
    }
    else // Right
    {
        const uint16_t lsb = data << 15;
        SetC(lsb);
        data >>= 1;
        data |= lsb;
    }

    SetN(data & 0x8000);
    SetZ(data == 0);
    SetV(0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::ROr()
{
    const uint8_t count = currentOpcode >> 9 & 0x0007;
    const uint8_t  size = currentOpcode >> 6 & 0x0003;
    const uint8_t   reg = currentOpcode & 0x0007;
    const uint8_t shift = currentOpcode & 0x0020 ? D[count] % 64 : (count ? count : 8);

    SetVC(0);

    uint32_t data, dataMask, msbMask, msbShift;
    if(size == 0) // Byte
    {
        dataMask = 0x000000FF;
        msbShift = 7;
    }
    else if(size == 1) // Word
    {
        dataMask = 0x0000FFFF;
        msbShift = 15;
    }
    else // Long
    {
        dataMask = 0xFFFFFFFF;
        msbShift = 31;
    }
    data = D[reg] & dataMask;
    msbMask = 1 << msbShift;

    if(currentOpcode & 0x0100) // Left
    {
        for(uint8_t i = shift; i > 0; i--)
        {
            const uint32_t msb = data >> msbShift & 1;
            SetC(msb);
            data <<= 1;
            data |= msb;
        }
    }
    else // Right
    {
        for(uint8_t i = shift; i > 0; i--)
        {
            const uint32_t lsb = data << msbShift & msbMask;
            SetC(lsb);
            data >>= 1;
            data |= lsb;
        }
    }

    SetN(data & msbMask);
    SetZ((data & dataMask) == 0);

    D[reg] &= ~dataMask;
    D[reg] |= data & dataMask;

    return 13 + 3 * shift;
}

uint16_t SCC68070::ROXm()
{
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 14;

    uint16_t data = GetWord(eamode, eareg, calcTime);
    if(currentOpcode & 0x0100) // Left
    {
        const uint16_t msb = data & 0x8000;
        data <<= 1;
        data |= GetX();
        SetXC(msb);
    }
    else // Right
    {
        const uint16_t lsb = data & 1;
        data >>= 1;
        data |= GetX() << 15;
        SetXC(lsb);
    }

    SetN(data & 0x8000);
    SetZ(data == 0);
    SetV(0);

    SetWord(lastAddress, data);

    return calcTime;
}

uint16_t SCC68070::ROXr()
{
    const uint8_t count = currentOpcode >> 9 & 0x0007;
    const uint8_t  size = currentOpcode >> 6 & 0x0003;
    const uint8_t   reg = currentOpcode & 0x0007;
    const uint8_t shift = currentOpcode & 0x0020 ? D[count] % 64 : (count ? count : 8);

    SetVC(0);

    uint32_t data, dataMask, msbMask, msbShift;
    if(size == 0) // Byte
    {
        dataMask = 0x000000FF;
        msbShift = 7;
    }
    else if(size == 1) // Word
    {
        dataMask = 0x0000FFFF;
        msbShift = 15;
    }
    else // Long
    {
        dataMask = 0xFFFFFFFF;
        msbShift = 31;
    }
    data = D[reg] & dataMask;
    msbMask = 1 << msbShift;

    if(currentOpcode & 0x0100) // Left
    {
        for(uint8_t i = shift; i > 0; i--)
        {
            const uint32_t msb = data & msbMask;
            data <<= 1;
            data |= GetX();
            SetXC(msb);
        }
    }
    else // Right
    {
        for(uint8_t i = shift; i > 0; i--)
        {
            const uint32_t lsb = data & 1;
            data >>= 1;
            data |= GetX() << msbShift;
            SetXC(lsb);
        }
    }

    SetN(data & msbMask);
    SetZ((data & dataMask) == 0);

    D[reg] &= ~dataMask;
    D[reg] |= data & dataMask;

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
        const uint16_t format = GetWord(ARIWPo(7, 2));

        if(format & 0xF000) // long format
        {
            A[7] += 26;
            calcTime = 146;
        }
    }
    else
        exceptions.push({PrivilegeViolation, 1});

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
    const uint8_t ry = currentOpcode >> 9 & 0x0007;
    const bool rm = currentOpcode >> 3 & 0x0001;
    const uint8_t rx = currentOpcode & 0x0007;
    uint16_t calcTime;

    uint8_t dst, src;
    if(rm) // Memory to Memory
    {
        src = PBCDToByte(GetByte(ARIWPr(rx, 1)));
        dst = PBCDToByte(GetByte(ARIWPr(ry, 1)));
        calcTime = 31;
    }
    else // Data register to Data register
    {
        src = PBCDToByte(D[rx] & 0x000000FF);
        dst = PBCDToByte(D[ry] & 0x000000FF);
        calcTime = 10;
    }

    int8_t result = dst - src - GetX();

    if(result != 0)
        SetZ(0);
    SetXC(result < 0);
    if(result < 0)
        result = 100 + result;

    if(rm)
        SetByte(lastAddress, byteToPBCD(result));
    else
    {
        D[ry] &= 0xFFFFFF00;
        D[ry] |= byteToPBCD(result);
    }

    return calcTime;
}

uint16_t SCC68070::Scc()
{
    const uint8_t condition = currentOpcode >> 8 & 0x000F;
    const uint8_t    eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t     eareg = currentOpcode & 0x0007;
    uint16_t calcTime = eamode ? 17 : 13;

    if((this->*ConditionalTests[condition])())
        SetByte(eamode, eareg, calcTime, 0xFF);
    else
        SetByte(eamode, eareg, calcTime, 0);

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
    {
        exceptions.push({PrivilegeViolation, 1});
        return 0;
    }

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

        if((uint8_t)src > (uint8_t)dst) SetXC(); else SetXC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] &= 0xFFFFFF00;
        D[reg] |= (uint8_t)(res & 0xFF);
    }
    else if(opmode == 1)
    {
        int16_t src = GetWord(eamode, eareg, calcTime);
        int16_t dst = D[reg] & 0x0000FFFF;
        int32_t res = dst - src;

        if((uint16_t)src > (uint16_t)dst) SetXC(); else SetXC(0);
        if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x8000) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] &= 0xFFFF0000;
        D[reg] |= (uint16_t)(res & 0xFFFF);
    }
    else if(opmode == 2)
    {
        int32_t src = GetLong(eamode, eareg, calcTime);
        int32_t dst = D[reg];
        int64_t res = dst - src;

        if((uint32_t)src > (uint32_t)dst) SetXC(); else SetXC(0);
        if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80000000) SetN(); else SetN(0);

        calcTime += 7;
        D[reg] = (uint32_t)(res & 0xFFFFFFFF);
    }
    else if(opmode == 4)
    {
        int8_t src = D[reg] & 0x000000FF;
        int8_t dst = GetByte(eamode, eareg, calcTime);
        int16_t res = dst - src;

        if((uint8_t)src > (uint8_t)dst) SetXC(); else SetXC(0);
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

        if((uint16_t)src > (uint16_t)dst) SetXC(); else SetXC(0);
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

        if((uint32_t)src > (uint32_t)dst) SetXC(); else SetXC(0);
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
        src = signExtend<int16_t, int32_t>(GetWord(eamode, eareg, calcTime));
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

        if((uint8_t)data > (uint8_t)dst) SetXC(); else SetXC(0);
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

        if((uint16_t)data > (uint16_t)dst) SetXC(); else SetXC(0);
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

        if((uint32_t)data > (uint32_t)dst) SetXC(); else SetXC(0);
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

        if((uint8_t)data > (uint8_t)dst) SetXC(); else SetXC(0);
        if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
        if(res == 0) SetZ(); else SetZ(0);
        if(res & 0x80) SetN(); else SetN(0);

        if(eamode == 0)
        {   D[eareg] &= 0xFFFFFF00; D[eareg] |= (uint8_t)(res & 0xFF); }
        else
        {   SetByte(lastAddress, res & 0xFF); calcTime += 4; }
    }
    else if(size == 1) // word
    {
        int16_t dst = GetWord(eamode, eareg, calcTime);
        int32_t res = dst - data;

        if(eamode != 1)
        {
            if((uint16_t)data > (uint16_t)dst) SetXC(); else SetXC(0);
            if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x8000) SetN(); else SetN(0);
        }

        if(eamode == 0)
        {   D[eareg] &= 0xFFFF0000; D[eareg] |= (uint16_t)(res & 0xFFFF); }
        else if(eamode == 1)
            A[eareg] = signExtend<int16_t, int32_t>(res);
        else
        {   SetWord(lastAddress, res & 0xFFFF); calcTime += 4; }
    }
    else // long
    {
        int32_t dst = GetLong(eamode, eareg, calcTime);
        int64_t res = dst - data;

        if(eamode != 1)
        {
            if((uint32_t)data > (uint32_t)dst) SetXC(); else SetXC(0);
            if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x80000000) SetN(); else SetN(0);
        }

        if(eamode == 0)
            D[eareg] = res & 0xFFFFFFFF;
        else if(eamode == 1)
            A[eareg] = res & 0xFFFFFFFF;
        else
        {   SetLong(lastAddress, res & 0xFFFFFFFF); calcTime += 8; }
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

            if((uint8_t)src + GetX() > (uint8_t)dst) SetXC(); else SetXC(0);
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

            if((uint16_t)src + GetX() > (uint16_t)dst) SetXC(); else SetXC(0);
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

            if((uint32_t)src + GetX() > (uint32_t)dst) SetXC(); else SetXC(0);
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

            if((uint8_t)src + GetX() > (uint8_t)dst) SetXC(); else SetXC(0);
            if(res < INT8_MIN || res > INT8_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x80) SetN(); else SetN(0);

            D[ry] &= 0xFFFFFF00;
            D[ry] |= (uint8_t)(res & 0xFF);
            calcTime = 7;
        }
        else if(size == 1) // word
        {
            int16_t src = D[rx] & 0x0000FFFF;
            int16_t dst = D[ry] & 0x0000FFFF;
            int32_t res = dst - src - GetX();

            if((uint16_t)src + GetX() > (uint16_t)dst) SetXC(); else SetXC(0);
            if(res < INT16_MIN || res > INT16_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x8000) SetN(); else SetN(0);

            D[ry] &= 0xFFFF0000;
            D[ry] |= (uint16_t)(res & 0xFFFF);
            calcTime = 7;
        }
        else // long
        {
            int32_t src = D[rx];
            int32_t dst = D[ry];
            int64_t res = dst - src - GetX();

            if((uint32_t)src + GetX() > (uint32_t)dst) SetXC(); else SetXC(0);
            if(res < INT32_MIN || res > INT32_MAX) SetV(); else SetV(0);
            if(res == 0) SetZ(); else SetZ(0);
            if(res & 0x80000000) SetN(); else SetN(0);

            D[ry] = (uint32_t)(res & 0xFFFFFFFF);
            calcTime = 7;
        }
    }

    return calcTime;
}

uint16_t SCC68070::SWAP()
{
    const uint8_t reg = currentOpcode & 0x0007;

    const uint16_t tmp = D[reg] >> 16 & 0x0000FFFF;
    D[reg] <<= 16;
    D[reg] |= tmp;

    SetN(D[reg] & 0x80000000);
    SetZ(D[reg] == 0);
    SetVC(0);

    return 7;
}

uint16_t SCC68070::TAS()
{
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 10;

    uint8_t data = GetByte(eamode, eareg, calcTime);
    SetN(data & 0x80);
    SetZ(data == 0);
    SetVC(0);

    data |= 0x80;
    if(eamode)
    {
        SetByte(lastAddress, data);
        calcTime++; // subtract one read cycle from effective address calculation (+5 - 4)
    }
    else
    {
        D[eareg] &= 0xFFFFFF00;
        D[eareg] |= data;
    }

    return calcTime;
}

uint16_t SCC68070::TRAP()
{
    uint8_t vec = currentOpcode & 0x000F;
    exceptions.push(SCC68070Exception(32 + vec, 2));
    return 0;
}

uint16_t SCC68070::TRAPV()
{
    if(!GetV())
        return 10;

    exceptions.push({TRAPVInstruction, 2});
    return 0;
}

uint16_t SCC68070::TST()
{
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7; // is calcTime = 7 for long a missclick in the datasheet?

    if(size == 0) // Byte
    {
        const uint8_t data = GetByte(eamode, eareg, calcTime);
        SetN(data & 0x80);
        SetZ(data == 0);
    }
    else if(size == 1) // Word
    {
        const uint16_t data = GetWord(eamode, eareg, calcTime);
        SetN(data & 0x8000);
        SetZ(data == 0);
    }
    else // Long
    {
        const uint32_t data = GetLong(eamode, eareg, calcTime);
        SetN(data & 0x80000000);
        SetZ(data == 0);
    }

    SetVC(0);

    return calcTime;
}

uint16_t SCC68070::UNLK()
{
    const uint8_t reg = currentOpcode & 0x0007;

    A[7] = A[reg];
    A[reg] = GetLong(ARIWPo(7, 4));

    return 15;
}
