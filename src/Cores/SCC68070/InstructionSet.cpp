#include "SCC68070.hpp"

void SCC68070::Exception(const uint8_t& vec)
{
    uint16_t sr = SR;
    SetS();

    SetLong(ARIWPr(7, 4), PC);
    SetWord(ARIWPr(7, 2), sr);

    PC = GetLong(vec * 4);
    SetS(0);
}

uint16_t SCC68070::UnknownInstruction()
{
#ifdef LOG_OPCODE
    errorLog("Unknow instruction");
    logln("Unknow instruction");
#endif // LOG_OPCODE
    return 0;
}

uint16_t SCC68070::Abcd()
{
#ifdef LOG_OPCODE
    std::cout << "ABCD";
#endif // LOG_OPCODE
    uint8_t Rx = (currentOpcode & 0x0E00) >> 9;
    uint8_t Ry = (currentOpcode & 0x0007);
    uint8_t result = 0;

    if(currentOpcode & 0x0008) // R/M = 1 : Memory to Memory
    {
        int32_t src = ARIWPr(Ry, 1);
        int32_t dst = ARIWPr(Rx, 1);
        SetByte(dst, convertPBCD(GetByte(src)) + convertPBCD(GetByte((dst)) + GetX()));
    }
    else // R/M = 0 : Data register to Data register
        result = convertPBCD(D[Rx] & 0xFF) + convertPBCD(D[Ry] & 0xFF) + GetX();

    if(result > 99)
        SetXC(1);
    else
        SetXC(0);
    if((D[Rx] = result) != 0)
        SetZ(0);

    return 14;
}

uint16_t SCC68070::Add()
{
#ifdef LOG_OPCODE
    std::cout << "ADD";
#endif // LOG_OPCODE
    uint8_t REGISTER = (currentOpcode & 0x0E00) >> 9;
    uint8_t   OPMODE = (currentOpcode & 0x01C0) >> 6;
    uint8_t   EAMODE = (currentOpcode & 0x0038) >> 3;
    uint8_t    EAREG = (currentOpcode & 0x0007);

    if(OPMODE){}

#ifdef DEBUG_OPCODE
    else
        errorLog("Wrong OPMODE in instruction ADD");
#endif // DEBUG_OPCODE

    return 14;
}

uint16_t SCC68070::Adda()
{
#ifdef LOG_OPCODE
    std::cout << "ADDA";
#endif // LOG_OPCODE

    return 14;
}

uint16_t SCC68070::Addi()
{
#ifdef LOG_OPCODE
    std::cout << "ADDI";
#endif // LOG_OPCODE
    uint8_t     SIZE = (currentOpcode & 0b0000000011000000) >> 6;/*
    uint8_t     MODE = (currentOpcode & 0b0000000000111000) >> 3;
    uint8_t REGISTER = (currentOpcode & 0b0000000000000111);*/
    if(SIZE == 0) // Byte
    {
    }
    else if(SIZE == 1) // Word
    {
    }
    else if(SIZE == 2) // Long
    {
    }
#ifdef DEBUG_OPCODE
    else
        std::cout << " WARNING: ADDI opcode " << std::hex << currentOpcode << ": SIZE doesn't match anything, instruction not executed";
#endif // DEBUG_OPCODE

    return 14;
}

uint16_t SCC68070::Addq()
{
#ifdef LOG_OPCODE
    std::cout << "ADDQ";
#endif // LOG_OPCODE

    return 14;
}

uint16_t SCC68070::Addx()
{
#ifdef LOG_OPCODE
    std::cout << "ADDX";
#endif // LOG_OPCODE
/*
    uint8_t     Rx = (currentOpcode & 0x0E00) >> 9;
    uint8_t   SIZE = (currentOpcode & 0x00C0) >> 6;
    uint8_t     Ry = (currentOpcode & 0x0007);
    int32_t result = 0;*/

    return 14;
}

uint16_t SCC68070::And()
{
#ifdef LOG_OPCODE
    log("AND ");
#endif // LOG_OPCODE
    uint16_t calcTime = 0;


    SetC(0);
    SetV(0);
    return calcTime;
}

uint16_t SCC68070::Andi()
{
#ifdef LOG_OPCODE
    std::cout << "ANDI";
#endif // LOG_OPCODE
    uint8_t SIZE = (currentOpcode & 0x00C0) >> 6;
    uint8_t MODE = (currentOpcode & 0x0038) >> 3;
    uint8_t EREG = (currentOpcode & 0x0007);
    if(SIZE == 0) // byte
    {
        int8_t data = GetNextWord() & 0xFF;
        if(MODE == 0)
        {
            uint8_t low = D[EREG] & 0xFF;
            D[EREG] &= 0xFFFFFF00;
            D[EREG] |= (D[EREG] & low) & 0xFF;
            if(D[EREG] & 0x80)
                SetN(1);
            else
                SetN(0);
            if(D[EREG] == 0)
                SetZ(1);
            else
                SetZ(0);
        }
        else if(MODE == 1)
        {
#ifdef DEBUG_OPCODE
            errorLog("Cannot use EA mode 1 in instruction ANDI");
#endif // DEBUG_OPCODE
        }
        else if(MODE == 2)
        {
            int8_t result = data & GetByte(A[EREG]);
            SetByte(A[EREG], result);
            if(result == 0)
                SetZ(1);
            else
                SetZ(0);
            if(result & 0x80)
                SetN(1);
            else
                SetN(0);
        }
    }
    else if(SIZE == 1) // word
    {

    }
    else if(SIZE == 2) // long
    {

    }
#ifdef DEBUG_OPCODE
    else
        errorLog("Wrong size in instruction ANDI");
#endif // DEBUG_OPCODE
    SetV(0);
    SetC(0);

    return 14;
}

uint16_t SCC68070::AsM()
{


    return 14;
}

uint16_t SCC68070::AsR()
{


    return 14;
}

uint16_t SCC68070::BCC() // Program Control
{
#ifdef LOG_OPCODE
    std::cout << "Bcc ";
#endif // LOG_OPCODE
    uint16_t calcTime;
    uint8_t condition = (currentOpcode & 0x0F00) >> 8;

    if(condition < 2) // True and False aren't available in Bcc
    {
        errorLog("Wrong condition in Bcc");
        return 14;
    }

    if((this->*ConditionalTests[condition])())
        if(currentOpcode & 0x00FF)
        {
            int8_t a = currentOpcode & 0x00FF;
            PC += a;
            calcTime = 13;
        }
        else
        {
            int16_t a = GetNextWord();
            PC += a;
            calcTime = 14;
        }
    else
        calcTime = 14;

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

uint16_t SCC68070::Bra() // Program Control
{
#ifdef LOG_OPCODE
    std::cout << "BRA ";
#endif // LOG_OPCODE
    uint16_t calcTime;
    int8_t a = currentOpcode & 0x00FF;
    if(a)
    {
        PC += a;
        calcTime = 13;
    }
    else
    {
        int16_t b = GetNextWord();
        PC += b;
        calcTime = 14;
    }
    return calcTime;
}

uint16_t SCC68070::Bset()
{



}

uint16_t SCC68070::Bsr() // Program Control
{
#ifdef LOG_OPCODE
    std::cout << "Bsr ";
#endif // LOG_OPCODE
    int32_t addr = ARIWPr(7, 4);
    int8_t a = currentOpcode & 0x00FF;
    if(a)
    {
        SetLong(addr, PC);
        PC += a;
    }
    else
    {
        int16_t b = GetNextWord();
        SetLong(addr, PC + 2);
        PC += b;
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
    uint16_t calcTime = 14; // arbitrary, only if errorLog macro is used

    if(MODE == 2)
    {   PC = A[REGISTER]; calcTime = 7; }
    else if(MODE == 5)
    {   PC = ARIWD(REGISTER); calcTime = 14; }
    else if(MODE == 6)
    {   PC = ARIWI8(REGISTER); calcTime = 17; }
    else if(MODE == 7)
    {
        PC = AM7(REGISTER);
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
        errorLog("Wrong addressing mode in JMP instruction");

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
    uint16_t calcTime = 26; // arbitrary, only if errorLog macro is used

    if(MODE == 2)
    {   PC = A[REGISTER]; calcTime = 18; }
    else if(MODE == 5)
    {   PC = ARIWD(REGISTER); calcTime = 25; }
    else if(MODE == 6)
    {   PC = ARIWI8(REGISTER); calcTime = 28; }
    else if(MODE == 7)
    {
        PC = AM7(REGISTER);
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
        errorLog("Wrong addressing mode in JMP instruction");

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
            errorLog("Wrong Regsiter for addressing mode 7 in Scc " << REGISTER);
    break;
    default:
        errorLog("Wrong addressing mode for Scc : " << MODE);
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

uint16_t SCC68070::Swap() // ok
{
#ifdef LOG_OPCODE
    log("SWAP ");
#endif // LOG_OPCODE
    uint8_t REGISTER = currentOpcode & 0x0007;
    uint32_t tmp = (D[REGISTER] & 0x0000FFFF) << 16;
    D[REGISTER] >>= 16;
    D[REGISTER] |= tmp;

    if(D[REGISTER]== 0)
        SetZ();
    else
        SetZ(0);
    if(D[REGISTER] & 0x80000000)
        SetZ();
    else
        SetZ(0);

    return 7;
}

uint16_t SCC68070::Tas() // ok
{
#ifdef LOG_OPCODE
    log("TAS ");
#endif // LOG_OPCODE
    uint8_t MODE = (currentOpcode & 0x0038) >> 3;
    uint8_t REGISTER = currentOpcode & 0x0007;
    uint16_t calcTime;
    SetV(0);
    SetC(0);

    if(!MODE)
    {
        calcTime = 10;
        if((D[REGISTER] & 0x000000FF) == 0)
            SetZ();
        else
            SetZ(0);
        if((D[REGISTER] & 0x00000080))
            SetN();
        else
            SetN(0);
        D[REGISTER] |= 0x00000080;
    }
    else
    {
        calcTime = 15;
        int8_t data = GetByte(MODE, REGISTER, calcTime);
        if(data == 0)
            SetZ();
        else
            SetZ(0);
        if(data & 0x80)
            SetN();
        else
            SetN(0);
        data |= 0x80;
        SetByte(lastAddress, data);
        calcTime -= 4;
    }
    return calcTime;
}

uint16_t SCC68070::Trap() // ok
{
#ifdef LOG_OPCODE
    log("TRAP ");
#endif // LOG_OPCODE
    uint8_t vec = (currentOpcode & 0x000F) + 32;
    Exception(vec);
    return 52; // Theorical, Exception processing clock periods
}

uint16_t SCC68070::Trapv() // ok
{
#ifdef LOG_OPCODE
    log("TRAPV ");
#endif // LOG_OPCODE
    if(GetV())
    {
        Exception(7);
        return 55;
    }
    else
        return 10;
}

uint16_t SCC68070::Tst() // ok
{
#ifdef LOG_OPCODE
    log("TST ");
#endif // LOG_OPCODE
#define COMPARE if(!data) SetZ(); else SetZ(0); if(data < 0) SetN(); else SetN(0);
    uint8_t size = (currentOpcode & 0x00C0) >> 6;
    uint8_t MODE = (currentOpcode & 0x0038) >> 3;
    uint8_t REGISTER = currentOpcode & 0x0007;
    uint16_t calcTime = 7;

    if(size == 0)
    {
        int8_t data = GetByte(MODE, REGISTER, calcTime);
        COMPARE
    }
    else if(size == 1)
    {
        int16_t data = GetWord(MODE, REGISTER, calcTime);
        COMPARE
    }
    else
    {
        int32_t data = GetLong(MODE, REGISTER, calcTime);
        COMPARE
    }

    SetC(0);
    SetV(0);
    return calcTime;
}

uint16_t SCC68070::Unlk() // ok
{
#ifdef LOG_OPCODE
    std::cout << "UNLK ";
#endif // LOG_OPCODE
    uint8_t REGISTER = currentOpcode & 0x0007;
    SP = A[REGISTER];
    A[REGISTER] = GetLong(SP);
    SP += 4;
    return 15;
}
