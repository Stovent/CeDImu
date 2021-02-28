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

template<typename T, typename VT, typename UVT>
static inline uint8_t add(const T src, const T dst, T* result, const VT min, const VT max, const UVT umax)
{
    const VT vres = (VT)src + (VT)dst;
    const T res = vres;

    uint8_t cc = 0;
    if(res < 0)
        cc |= 0b01000;
    if(res == 0)
        cc |= 0b00100;
    if(vres < min || vres > max)
        cc |= 0x00010;
    if((UVT)vres > umax)
        cc |= 0b10001;

    *result = res;

    return cc;
}

uint16_t SCC68070::ADD()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t opmode = currentOpcode >> 8 & 0x0001;
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 0) // Byte
    {
        const int8_t src = opmode ? D[reg] : GetByte(eamode, eareg, calcTime);
        const int8_t dst = opmode ? GetByte(eamode, eareg, calcTime) : D[reg];
        int8_t res;

        SR &= SR_UPPER_MASK;
        SR |= add<int8_t, int16_t, uint16_t>(src, dst, &res, INT8_MIN, INT8_MAX, UINT8_MAX);

        if(opmode)
        {
            SetByte(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[reg] &= 0xFFFFFF00;
            D[reg] |= (uint8_t)res;
        }
    }
    else if(size == 1) // Word
    {
        const int16_t src = opmode ? D[reg] : GetWord(eamode, eareg, calcTime);
        const int16_t dst = opmode ? GetWord(eamode, eareg, calcTime) : D[reg];
        int16_t res;

        SR &= SR_UPPER_MASK;
        SR |= add<int16_t, int32_t, uint32_t>(src, dst, &res, INT16_MIN, INT16_MAX, UINT16_MAX);

        if(opmode)
        {
            SetWord(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[reg] &= 0xFFFF0000;
            D[reg] |= (uint16_t)res;
        }
    }
    else // Long
    {
        const int32_t src = opmode ? D[reg] : GetLong(eamode, eareg, calcTime);
        const int32_t dst = opmode ? GetLong(eamode, eareg, calcTime) : D[reg];
        int32_t res;

        SR &= SR_UPPER_MASK;
        SR |= add<int32_t, int64_t, uint64_t>(src, dst, &res, INT32_MIN, INT32_MAX, UINT32_MAX);

        if(opmode)
        {
            SetLong(lastAddress, res);
            calcTime += 8;
        }
        else
        {
            D[reg] = res;
        }
    }

    return calcTime;
}

uint16_t SCC68070::ADDA()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t   size = currentOpcode >> 8 & 0x0001;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    int32_t src;
    if(size) // Long
        src = GetLong(eamode, eareg, calcTime);
    else // Word
        src = signExtend<int16_t, int32_t>(GetWord(eamode, eareg, calcTime));

    A[reg] += src;

    return calcTime;
}

uint16_t SCC68070::ADDI()
{
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 14;

    if(size == 0) // Byte
    {
        const int8_t data = GetNextWord() & 0x00FF;
        const int8_t  dst = GetByte(eamode, eareg, calcTime);
        int8_t res;

        SR &= SR_UPPER_MASK;
        SR |= add<int8_t, int16_t, uint16_t>(data, dst, &res, INT8_MIN, INT8_MAX, UINT8_MAX);

        if(eamode)
        {
            SetByte(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFFFF00;
            D[eareg] |= (uint8_t)res;
        }
    }
    else if(size == 1) // Word
    {
        const int16_t data = GetNextWord();
        const int16_t  dst = GetWord(eamode, eareg, calcTime);
        int16_t res;

        SR &= SR_UPPER_MASK;
        SR |= add<int16_t, int32_t, uint32_t>(data, dst, &res, INT16_MIN, INT16_MAX, UINT16_MAX);

        if(eamode)
        {
            SetWord(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFF0000;
            D[eareg] |= (uint16_t)res;
        }
    }
    else // Long
    {
        const int32_t data = (uint32_t)GetNextWord() << 16 | GetNextWord();
        const int32_t  dst = GetLong(eamode, eareg, calcTime);
        int32_t res;

        SR &= SR_UPPER_MASK;
        SR |= add<int32_t, int64_t, uint64_t>(data, dst, &res, INT32_MIN, INT32_MAX, UINT32_MAX);

        if(eamode)
        {
            SetLong(lastAddress, res);
            calcTime += 12;
        }
        else
        {
            D[eareg] = res;
            calcTime += 4;
        }
    }

    return calcTime;
}

uint16_t SCC68070::ADDQ()
{
    const uint8_t   data = currentOpcode & 0x0E00 ? (currentOpcode >> 9 & 0x0007) : 8;
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(eamode == 1)
    {
        A[eareg] += data;
        return 7;
    }

    if(size == 0) // Byte
    {
        const int8_t dst = GetByte(eamode, eareg, calcTime);
        int8_t res;

        SR &= SR_UPPER_MASK;
        SR |= add<int8_t, int16_t, uint16_t>(data, dst, &res, INT8_MIN, INT8_MAX, UINT8_MAX);

        if(eamode)
        {
            SetByte(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFFFF00;
            D[eareg] |= (uint8_t)res;
        }
    }
    else if(size == 1) // Word
    {
        const int16_t dst = GetWord(eamode, eareg, calcTime);
        int16_t res;

        SR &= SR_UPPER_MASK;
        SR |= add<int16_t, int32_t, uint32_t>(data, dst, &res, INT16_MIN, INT16_MAX, UINT16_MAX);

        if(eamode)
        {
            SetWord(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFF0000;
            D[eareg] |= (uint16_t)res;
        }
    }
    else // Long
    {
        const int32_t dst = GetLong(eamode, eareg, calcTime);
        int32_t res;

        SR &= SR_UPPER_MASK;
        SR |= add<int32_t, int64_t, uint64_t>(data, dst, &res, INT32_MIN, INT32_MAX, UINT32_MAX);

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

uint16_t SCC68070::ADDX()
{
    const uint8_t   rx = currentOpcode >> 9 & 0x0007;
    const uint8_t size = currentOpcode >> 6 & 0x0003;
    const uint8_t   rm = currentOpcode >> 3 & 0x0001;
    const uint8_t   ry = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 0) // Byte
    {
        const int8_t src = rm ? GetByte(ARIWPr(ry, 1)) : D[ry];
        const int8_t dst = rm ? GetByte(ARIWPr(rx, 1)) : D[rx];
        int8_t res;

        const uint8_t cc = add<int8_t, int16_t, uint16_t>(src + GetX(), dst, &res, INT8_MIN, INT8_MAX, UINT8_MAX);
        SR &= SR_UPPER_MASK | 0b00100;
        SR |= cc & 0b11011;
        if(!(cc & 0b00100))
            SetZ(0);

        if(rm)
        {
            SetByte(A[rx], res);
            calcTime = 28;
        }
        else
        {
            D[rx] &= 0xFFFFFF00;
            D[rx] |= (uint8_t)res;
        }
    }
    else if(size == 1) // Word
    {
        const int16_t src = rm ? GetWord(ARIWPr(ry, 2)) : D[ry];
        const int16_t dst = rm ? GetWord(ARIWPr(rx, 2)) : D[rx];
        int16_t res;

        const uint8_t cc = add<int16_t, int32_t, uint32_t>(src + GetX(), dst, &res, INT16_MIN, INT16_MAX, UINT16_MAX);
        SR &= SR_UPPER_MASK | 0b00100;
        SR |= cc & 0b11011;
        if(!(cc & 0b00100))
            SetZ(0);

        if(rm)
        {
            SetWord(A[rx], res);
            calcTime = 28;
        }
        else
        {
            D[rx] &= 0xFFFF0000;
            D[rx] |= (uint16_t)res;
        }
    }
    else // Long
    {
        const int32_t src = rm ? GetLong(ARIWPr(ry, 4)) : D[ry];
        const int32_t dst = rm ? GetLong(ARIWPr(rx, 4)) : D[rx];
        int32_t res;

        const uint8_t cc = add<int32_t, int64_t, uint64_t>(src + GetX(), dst, &res, INT32_MIN, INT32_MAX, UINT32_MAX);
        SR &= SR_UPPER_MASK | 0b00100;
        SR |= cc & 0b11011;
        if(!(cc & 0b00100))
            SetZ(0);

        if(rm)
        {
            SetLong(A[rx], res);
            calcTime = 40;
        }
        else
        {
            D[rx] = res;
        }
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
    const uint16_t data = SR_UPPER_MASK | (GetNextWord() & 0x001F);
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
        bit = GetNextWord() & 0x01FF; // TODO: error in the datasheet ?
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
    const uint32_t pc = PC;
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

template<typename T, typename UT, typename VT>
static inline uint8_t cmp(const T src, const T dst, const VT min, const VT max)
{
    const VT vres = (VT)dst - (VT)src;
    const T res = vres;

    uint8_t cc = 0;
    if(res < 0)
        cc |= 0b01000;
    if(res == 0)
        cc |= 0b00100;
    if(vres < min || vres > max)
        cc |= 0x00010;
    if((UT)src > (UT)dst)
        cc |= 0b00001;

    return cc;
}

uint16_t SCC68070::CMP()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 0) // Byte
    {
        const int8_t src = GetByte(eamode, eareg, calcTime);
        const int8_t dst = D[reg];

        SR &= SR_UPPER_MASK | 0x10; // Keep X bit
        SR |= cmp<int8_t, uint8_t, int16_t>(src, dst, INT8_MIN, INT8_MAX);
    }
    else if(size == 1) // Word
    {
        const int16_t src = GetWord(eamode, eareg, calcTime);
        const int16_t dst = D[reg];

        SR &= SR_UPPER_MASK | 0x10;
        SR |= cmp<int16_t, uint16_t, int32_t>(src, dst, INT16_MIN, INT16_MAX);
    }
    else // Long
    {
        const int32_t src = GetLong(eamode, eareg, calcTime);
        const int32_t dst = D[reg];

        SR &= SR_UPPER_MASK | 0x10;
        SR |= cmp<int32_t, uint32_t, int64_t>(src, dst, INT32_MIN, INT32_MAX);
    }

    return calcTime;
}

uint16_t SCC68070::CMPA()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t   size = currentOpcode >> 8 & 0x0001;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    int32_t src;
    if(size) // Long
        src = GetLong(eamode, eareg, calcTime);
    else // Word
        src = signExtend<int16_t, int32_t>(GetWord(eamode, eareg, calcTime));

    const int64_t vres = signExtend<int32_t, int64_t>(A[reg]) - src;
    const int32_t res = vres;

    SetN(res < 0);
    SetZ(res == 0);
    SetV(vres < INT32_MIN || vres > INT32_MAX);
    SetC((uint32_t)src > A[reg]);

    return calcTime;
}

uint16_t SCC68070::CMPI()
{
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 14;

    if(size == 0) // Byte
    {
        const int8_t data = GetNextWord() & 0x00FF;
        const int8_t  dst = GetByte(eamode, eareg, calcTime);

        SR &= SR_UPPER_MASK | 0x10; // Keep X bit
        SR |= cmp<int8_t, uint8_t, int16_t>(data, dst, INT8_MIN, INT8_MAX);
    }
    else if(size == 1) // Word
    {
        const int16_t data = GetNextWord();
        const int16_t  dst = GetWord(eamode, eareg, calcTime);

        SR &= SR_UPPER_MASK | 0x10;
        SR |= cmp<int16_t, uint16_t, int32_t>(data, dst, INT16_MIN, INT16_MAX);
    }
    else // Long
    {
        const int32_t data = (uint32_t)GetNextWord() << 16 | GetNextWord();
        const int32_t  dst = GetLong(eamode, eareg, calcTime);

        SR &= SR_UPPER_MASK | 0x10;
        SR |= cmp<int32_t, uint32_t, int64_t>(data, dst, INT32_MIN, INT32_MAX);
        calcTime += 4;
    }

    return calcTime;
}

uint16_t SCC68070::CMPM()
{
    const uint8_t   ax = currentOpcode >> 9 & 0x0007;
    const uint8_t size = currentOpcode >> 6 & 0x0003;
    const uint8_t   ay = currentOpcode & 0x0007;

    if(size == 0) // Byte
    {
        const int8_t src = GetByte(ARIWPo(ay, 1));
        const int8_t dst = GetByte(ARIWPo(ax, 1));

        SR &= SR_UPPER_MASK | 0x10; // Keep X bit
        SR |= cmp<int8_t, uint8_t, int16_t>(src, dst, INT8_MIN, INT8_MAX);
    }
    else if(size == 1) // Word
    {
        const int16_t src = GetWord(ARIWPo(ay, 2));
        const int16_t dst = GetWord(ARIWPo(ax, 2));

        SR &= SR_UPPER_MASK | 0x10;
        SR |= cmp<int16_t, uint16_t, int32_t>(src, dst, INT16_MIN, INT16_MAX);
    }
    else // Long
    {
        const int32_t src = GetLong(ARIWPo(ay, 4));
        const int32_t dst = GetLong(ARIWPo(ax, 4));

        SR &= SR_UPPER_MASK | 0x10;
        SR |= cmp<int32_t, uint32_t, int64_t>(src, dst, INT32_MIN, INT32_MAX);
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
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 169; // LMAO

    const int16_t src = GetWord(eamode, eareg, calcTime);
    if(src == 0)
    {
        exceptions.push({ZeroDivide, 2});
        return 7; // Arbitrary
    }

    const int32_t dst = D[reg];
    const int32_t q = dst / src;

    if(q < INT16_MIN || q > INT16_MAX)
    {
        SetV();
        return calcTime; // TODO: accuracy of this.
    }

    const int16_t r = dst % src;

    SetN(q < 0);
    SetZ(q == 0);
    SetVC(0);

    D[reg]  = (uint16_t)r << 16;
    D[reg] |= (uint16_t)(int16_t)q;

    return calcTime;
}

uint16_t SCC68070::DIVU()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 130;

    const uint16_t src = GetWord(eamode, eareg, calcTime);
    if(src == 0)
    {
        exceptions.push({ZeroDivide, 2});
        return 7; // Arbitrary
    }

    const uint32_t q = D[reg] / src;
    if(q > UINT16_MAX)
    {
        SetV();
        return calcTime; // TODO: accuracy of this.
    }

    const uint16_t r = D[reg] % src;

    SetN(q & 0x8000);
    SetZ(q == 0);
    SetVC(0);

    D[reg]  = r << 16;
    D[reg] |= q & 0x0000FFFF;

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
    return 0;
}

uint16_t SCC68070::JMP()
{
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = (eamode == 7 && eareg <= 1) ? 6 : 3;

    PC = GetEffectiveAddress(eamode, eareg, 1, calcTime); // 1 to get byte/word calculation time

    return calcTime;
}

uint16_t SCC68070::JSR()
{
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = (eamode == 7 && eareg <= 1) ? 17 : 14;

    // write PC first so the updated stack pointer can be used as effective address
    SetLong(ARIWPr(7, 4), PC + (eamode == 7 ? (eareg == 1 ? 4 : 2) : (eamode >= 5 ? 2 : 0)));

    PC = GetEffectiveAddress(eamode, eareg, 1, calcTime); // 1 to get byte/word calculation time

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
    const uint8_t    size = currentOpcode >> 12 & 0x0003;
    const uint8_t  dstreg = currentOpcode >> 9 & 0x0007;
    const uint8_t dstmode = currentOpcode >> 6 & 0x0007;
    const uint8_t srcmode = currentOpcode >> 3 & 0x0007;
    const uint8_t  srcreg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 1) // Byte
    {
        const uint8_t src = GetByte(srcmode, srcreg, calcTime);

        SetN(src & 0x80);
        SetZ(src == 0);

        SetByte(dstmode, dstreg, calcTime, src);
    }
    else if(size == 3) // Word
    {
        const uint16_t src = GetWord(srcmode, srcreg, calcTime);

        SetN(src & 0x8000);
        SetZ(src == 0);

        SetWord(dstmode, dstreg, calcTime, src);
    }
    else // Long
    {
        const uint32_t src = GetLong(srcmode, srcreg, calcTime);

        SetN(src & 0x80000000);
        SetZ(src == 0);

        SetLong(dstmode, dstreg, calcTime, src);
    }

    SetVC(0);

    return calcTime;
}

uint16_t SCC68070::MOVEA()
{
    const uint8_t   size = currentOpcode >> 12 & 0x0003;
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 3) // Word
    {
        A[reg] = signExtend<int16_t, int32_t>(GetWord(eamode, eareg, calcTime));
    }
    else // Long
    {
        A[reg] = GetLong(eamode, eareg, calcTime);
    }

    return calcTime;
}

uint16_t SCC68070::MOVECCR()
{
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 10;

    const uint16_t data = GetWord(eamode, eareg, calcTime) & 0x001F;
    SR &= SR_UPPER_MASK;
    SR |= data;

    return calcTime;
}

uint16_t SCC68070::MOVEfSR() // Should not be used according to the Green Book Chapter VI.2.2.2
{
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = eamode ? 11 : 7;

    SetWord(eamode, eareg, calcTime, SR & 0xA71F);

    return calcTime;
}

uint16_t SCC68070::MOVESR()
{
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 10;

    if(!GetS())
    {
        exceptions.push({PrivilegeViolation, 1});
        return 0;
    }

    SR = GetWord(eamode, eareg, calcTime) & 0xA71F;

    if(!GetS()) // S bit changes from supervisor to user
    {
        SSP = A[7];
        A[7] = USP;
    }

    return calcTime;
}

uint16_t SCC68070::MOVEUSP()
{
    const uint8_t reg = currentOpcode & 0x0007;

    if(!GetS())
    {
        exceptions.push({PrivilegeViolation, 1});
        return 0;
    }

    if(currentOpcode & 0x0008) // USP to An
    {
        A[reg] = USP;
    }
    else // An to USP
    {
        USP = A[reg];
    }

    return 7;
}

uint16_t SCC68070::MOVEM()
{
    const uint8_t     dr = currentOpcode >> 10 & 0x0001;
    const uint8_t   size = currentOpcode >> 6 & 0x0001;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t list = GetNextWord();
    uint16_t calcTime = ((eamode == 7 && eareg <= 1) || eamode <= 4) ? 19 : 16;
    if(dr)
        calcTime += 3;
    if(size)
        calcTime -= 4;

    uint8_t count = 0;
    const int gap = size ? 4 : 2;
    const uint32_t initialReg = A[eareg];
    uint32_t addr = GetEffectiveAddress(eamode, eareg, gap, calcTime);
    if(eamode == 4)
    {
        A[eareg] = initialReg;
        for(int i = 7; i >= 0; i--)
        {
            if(list & 1)
            {
                size ? SetLong(addr, A[i]) : SetWord(addr, A[i]);
                addr -= gap;
                count++;
            }
            list >>= 1;
        }

        for(int i = 7; i >= 0; i--)
        {
            if(list & 1)
            {
                size ? SetLong(addr, D[i]) : SetWord(addr, D[i]);
                addr -= gap;
                count++;
            }
            list >>= 1;
        }

        A[eareg] = addr + gap;
    }
    else
    {
        for(int i = 0; i < 8; i++)
        {
            if(list & 1)
            {
                if(dr) // Memory to register
                    D[i] = size ? GetLong(addr) : signExtend<int16_t, int32_t>(GetWord(addr));
                else // Register to memory
                    size ? SetLong(addr, D[i]) : SetWord(addr, D[i]);
                addr += gap;
                count++;
            }
            list >>= 1;
        }

        for(int i = 0; i < 8; i++)
        {
            if(list & 1)
            {
                if(dr) // Memory to register
                    A[i] = size ? GetLong(addr) : signExtend<int16_t, int32_t>(GetWord(addr));
                else // Register to memory
                    size ? SetLong(addr, A[i]) : SetWord(addr, A[i]);
                addr += gap;
                count++;
            }
            list >>= 1;
        }

        if(eamode == 3)
            A[eareg] = addr;
    }

    return calcTime + count * (size ? 11 : 7);
}

uint16_t SCC68070::MOVEP()
{
    const uint8_t   dreg = currentOpcode >> 9 & 0x0007;
    const uint8_t opmode = currentOpcode >> 7 & 0x0001;
    const uint8_t   size = currentOpcode >> 6 & 0x0001;
    const uint8_t   areg = currentOpcode & 0x0007;
    uint16_t calcTime;

    uint32_t addr = ARIWD(areg);
    int shift = size ? 24 : 8;
    if(opmode) // Register to memory
    {
        for(; shift >= 0; shift -= 8, addr += 2)
        {
            SetByte(addr, D[dreg] >> shift);
        }

        calcTime = size ? 39 : 25;
    }
    else // Memory to register
    {
        D[dreg] &= size ? 0 : 0xFFFF0000;

        for(; shift >= 0; shift -= 8, addr += 2)
        {
            D[dreg] |= (uint32_t)GetByte(addr) << shift;
        }

        calcTime = size ? 36 : 22;
    }

    return calcTime;
}

uint16_t SCC68070::MOVEQ()
{
    const uint8_t  reg = currentOpcode >> 9 & 0x0007;
    const uint8_t data = currentOpcode & 0x00FF;

    SetN(data & 0x80);
    SetZ(data == 0);
    SetVC(0);

    D[reg] = signExtend<int8_t, int32_t>(data);
    return 7;
}

uint16_t SCC68070::MULS()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 76;

    int16_t src = GetWord(eamode, eareg, calcTime);
    int16_t dst = D[reg] & 0x0000FFFF;
    D[reg] = (int32_t)src * dst;

    SetN(D[reg] & 0x80000000);
    SetZ(D[reg] == 0);
    SetVC(0);

    return calcTime;
}

uint16_t SCC68070::MULU()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 76;

    uint16_t src = GetWord(eamode, eareg, calcTime);
    uint16_t dst = D[reg] & 0x0000FFFF;
    D[reg] = (uint32_t)src * dst;

    SetN(D[reg] & 0x80000000);
    SetZ(D[reg] == 0);
    SetVC(0);

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
    if(!GetS())
    {
        exceptions.push({PrivilegeViolation, 1});
        return 0;
    }

    uint16_t calcTime = 39;
    SR = GetWord(ARIWPo(7, 2));
    PC = GetLong(ARIWPo(7, 4));
    const uint16_t format = GetWord(ARIWPo(7, 2));

    if((format & 0xF000) == 0xF000) // long format
    {
        A[7] += 26;
        calcTime = 146;
    }
    else if((format & 0xF000) != 0) // Format error
    {
        exceptions.push({FormatError, 2});
    }

    return calcTime;
}

uint16_t SCC68070::RTR()
{
    SR &= SR_UPPER_MASK;
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

uint16_t SCC68070::STOP() // TODO: correctly implement it.
{
    const uint16_t data = GetNextWord();

    if(!GetS())
    {
        exceptions.push({PrivilegeViolation, 1});
        return 0;
    }

    SR = data;
    if(!GetS()) // S bit changed from supervisor to user
    {
        SSP = A[7];
        A[7] = USP;
    }
    SR &= 0xA71F; // Set all unimplemented bytes to 0.

    return 13;
}

template<typename T, typename UT, typename VT>
static inline uint8_t sub(const T src, const T dst, T* result, const VT min, const VT max)
{
    const VT vres = (VT)dst - (VT)src;
    const T res = vres;

    uint8_t cc = 0;
    if(res < 0)
        cc |= 0b01000;
    if(res == 0)
        cc |= 0b00100;
    if(vres < min || vres > max)
        cc |= 0x00010;
    if((UT)src > (UT)dst)
        cc |= 0b10001;

    *result = res;

    return cc;
}

uint16_t SCC68070::SUB()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t opmode = currentOpcode >> 8 & 0x0001;
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 0) // Byte
    {
        const int8_t src = opmode ? D[reg] : GetByte(eamode, eareg, calcTime);
        const int8_t dst = opmode ? GetByte(eamode, eareg, calcTime) : D[reg];
        int8_t res;

        SR &= SR_UPPER_MASK;
        SR |= sub<int8_t, uint8_t, int16_t>(src, dst, &res, INT8_MIN, INT8_MAX);

        if(opmode)
        {
            SetByte(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[reg] &= 0xFFFFFF00;
            D[reg] |= (uint8_t)res;
        }
    }
    else if(size == 1) // Word
    {
        const int16_t src = opmode ? D[reg] : GetWord(eamode, eareg, calcTime);
        const int16_t dst = opmode ? GetWord(eamode, eareg, calcTime) : D[reg];
        int16_t res;

        SR &= SR_UPPER_MASK;
        SR |= sub<int16_t, uint16_t, int32_t>(src, dst, &res, INT16_MIN, INT16_MAX);

        if(opmode)
        {
            SetWord(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[reg] &= 0xFFFF0000;
            D[reg] |= (uint16_t)res;
        }
    }
    else // Long
    {
        const int32_t src = opmode ? D[reg] : GetLong(eamode, eareg, calcTime);
        const int32_t dst = opmode ? GetLong(eamode, eareg, calcTime) : D[reg];
        int32_t res;

        SR &= SR_UPPER_MASK;
        SR |= sub<int32_t, uint32_t, int64_t>(src, dst, &res, INT32_MIN, INT32_MAX);

        if(opmode)
        {
            SetLong(lastAddress, res);
            calcTime += 8;
        }
        else
        {
            D[reg] = res;
        }
    }

    return calcTime;
}

uint16_t SCC68070::SUBA()
{
    const uint8_t    reg = currentOpcode >> 9 & 0x0007;
    const uint8_t   size = currentOpcode >> 8 & 0x0001;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    int32_t src;
    if(size) // Long
        src = GetLong(eamode, eareg, calcTime);
    else // Word
        src = signExtend<int16_t, int32_t>(GetWord(eamode, eareg, calcTime));

    A[reg] -= src;

    return calcTime;
}

uint16_t SCC68070::SUBI()
{
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 14;

    if(size == 0) // Byte
    {
        const int8_t data = GetNextWord() & 0x00FF;
        const int8_t  dst = GetByte(eamode, eareg, calcTime);
        int8_t res;

        SR &= SR_UPPER_MASK;
        SR |= sub<int8_t, uint8_t, int16_t>(data, dst, &res, INT8_MIN, INT8_MAX);

        if(eamode)
        {
            SetByte(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFFFF00;
            D[eareg] |= (uint8_t)res;
        }
    }
    else if(size == 1) // Word
    {
        const int16_t data = GetNextWord();
        const int16_t  dst = GetWord(eamode, eareg, calcTime);
        int16_t res;

        SR &= SR_UPPER_MASK;
        SR |= sub<int16_t, uint16_t, int32_t>(data, dst, &res, INT16_MIN, INT16_MAX);

        if(eamode)
        {
            SetWord(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFF0000;
            D[eareg] |= (uint16_t)res;
        }
    }
    else // Long
    {
        const int32_t data = (uint32_t)GetNextWord() << 16 | GetNextWord();
        const int32_t  dst = GetLong(eamode, eareg, calcTime);
        int32_t res;

        SR &= SR_UPPER_MASK;
        SR |= sub<int32_t, uint32_t, int64_t>(data, dst, &res, INT32_MIN, INT32_MAX);

        if(eamode)
        {
            SetLong(lastAddress, res);
            calcTime += 12;
        }
        else
        {
            D[eareg] = res;
            calcTime += 4;
        }
    }

    return calcTime;
}

uint16_t SCC68070::SUBQ()
{
    const uint8_t   data = currentOpcode & 0x0E00 ? (currentOpcode >> 9 & 0x0007) : 8;
    const uint8_t   size = currentOpcode >> 6 & 0x0003;
    const uint8_t eamode = currentOpcode >> 3 & 0x0007;
    const uint8_t  eareg = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(eamode == 1)
    {
        A[eareg] -= data;
        return 7;
    }

    if(size == 0) // Byte
    {
        const int8_t dst = GetByte(eamode, eareg, calcTime);
        int8_t res;

        SR &= SR_UPPER_MASK;
        SR |= sub<int8_t, uint8_t, int16_t>(data, dst, &res, INT8_MIN, INT8_MAX);

        if(eamode)
        {
            SetByte(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFFFF00;
            D[eareg] |= (uint8_t)res;
        }
    }
    else if(size == 1) // Word
    {
        const int16_t dst = GetWord(eamode, eareg, calcTime);
        int16_t res;

        SR &= SR_UPPER_MASK;
        SR |= sub<int16_t, uint16_t, int32_t>(data, dst, &res, INT16_MIN, INT16_MAX);

        if(eamode)
        {
            SetWord(lastAddress, res);
            calcTime += 4;
        }
        else
        {
            D[eareg] &= 0xFFFF0000;
            D[eareg] |= (uint16_t)res;
        }
    }
    else // Long
    {
        const int32_t dst = GetLong(eamode, eareg, calcTime);
        int32_t res;

        SR &= SR_UPPER_MASK;
        SR |= sub<int32_t, uint32_t, int64_t>(data, dst, &res, INT32_MIN, INT32_MAX);

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

uint16_t SCC68070::SUBX()
{
    const uint8_t   ry = currentOpcode >> 9 & 0x0007;
    const uint8_t size = currentOpcode >> 6 & 0x0003;
    const uint8_t   rm = currentOpcode >> 3 & 0x0001;
    const uint8_t   rx = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 0) // Byte
    {
        const int8_t src = rm ? GetByte(ARIWPr(rx, 1)) : D[rx];
        const int8_t dst = rm ? GetByte(ARIWPr(ry, 1)) : D[ry];
        int8_t res;

        const uint8_t cc = sub<int8_t, uint8_t, int16_t>(src + GetX(), dst, &res, INT8_MIN, INT8_MAX);
        SR &= SR_UPPER_MASK | 0b00100; // Z may be unchanged
        SR |= cc & 0b11011;
        if(!(cc & 0b00100))
            SetZ(0);

        if(rm)
        {
            SetByte(A[ry], res);
            calcTime = 28;
        }
        else
        {
            D[ry] &= 0xFFFFFF00;
            D[ry] |= (uint8_t)res;
        }
    }
    else if(size == 1) // Word
    {
        const int16_t src = rm ? GetWord(ARIWPr(rx, 2)) : D[rx];
        const int16_t dst = rm ? GetWord(ARIWPr(ry, 2)) : D[ry];
        int16_t res;

        const uint8_t cc = sub<int16_t, uint16_t, int32_t>(src + GetX(), dst, &res, INT16_MIN, INT16_MAX);
        SR &= SR_UPPER_MASK | 0b00100; // Z may be unchanged
        SR |= cc & 0b11011;
        if(!(cc & 0b00100))
            SetZ(0);

        if(rm)
        {
            SetWord(A[ry], res);
            calcTime = 28;
        }
        else
        {
            D[ry] &= 0xFFFF0000;
            D[ry] |= (uint16_t)res;
        }
    }
    else // Long
    {
        const int32_t src = rm ? GetLong(ARIWPr(rx, 4)) : D[rx];
        const int32_t dst = rm ? GetLong(ARIWPr(ry, 4)) : D[ry];
        int32_t res;

        const uint8_t cc = sub<int32_t, uint32_t, int64_t>(src + GetX(), dst, &res, INT32_MIN, INT32_MAX);
        SR &= SR_UPPER_MASK | 0b00100; // Z may be unchanged
        SR |= cc & 0b11011;
        if(!(cc & 0b00100))
            SetZ(0);

        if(rm)
        {
            SetLong(A[ry], res);
            calcTime = 40;
        }
        else
        {
            D[ry] = res;
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
    const uint8_t vector = currentOpcode & 0x000F;
    exceptions.push({uint8_t(vector + 32), 2});
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
