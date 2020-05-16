#include "SCC68070.hpp"

#include <cstring>

SCC68070::SCC68070(VDSC* gpu, const uint32_t clockFrequency) : disassembledInstructions(), ILUT()
{
    vdsc = gpu;
    disassemble = false;

    Execute = &SCC68070::Interpreter;
    internal = new uint8_t[SCC68070Peripherals::Size];
    instructionCount = 0;
    cycleDelay = (1.0L / clockFrequency) * 1000000000;

    OPEN_LOG(out, "SCC68070.txt")
    OPEN_LOG(instruction, "instructions.txt")
    OPEN_LOG(uart_out, "uart_out.txt")
    uart_in.open("uart_in", std::ios::binary | std::ios::in);

    GenerateInstructionSet();
    RebootCore();

    SET_TX_READY
    SET_RX_READY
}

SCC68070::~SCC68070()
{
    delete[] internal;
}

void SCC68070::RebootCore()
{
    run = false;
    LOG(out << "RESET" << std::endl; instruction << "RESET" << std::endl;)
    cycleCount = 146;
    totalCycleCount = 146;
    currentOpcode = 0;
    currentPC = 0;
    lastAddress = 0;
    memset(internal, 0, SCC68070Peripherals::Size);
    for(uint8_t i = 0; i < 8; i++)
    {
        D[i] = 0;
        A[i] = 0;
    }
    disassembledInstructions.clear();
    ResetOperation();
}

void SCC68070::ResetOperation()
{
    vdsc->MemorySwap();
    SSP = vdsc->GetLong(0);
    A[7] = SSP;
    PC = vdsc->GetLong(4);
    SR = 0x2700;
    USP = 0;
}

void SCC68070::SingleStep()
{
    if(!run)
        (this->*Execute)(false);
}

void SCC68070::Run()
{
    (this->*Execute)(true);
}

uint16_t SCC68070::GetNextWord(const uint8_t flags)
{
    uint16_t opcode = vdsc->GetWord(PC, flags);
    PC += 2;
    return opcode;
}

void SCC68070::SetCCR(const uint8_t X, const uint8_t N, const uint8_t Z, const uint8_t V, const uint8_t C) // use the define UNCHANGED to not change the value of a bit
{
    if(X != UNCHANGED)
        SetX(X);
    if(N != UNCHANGED)
        SetN(N);
    if(Z != UNCHANGED)
        SetZ(Z);
    if(V != UNCHANGED)
        SetV(V);
    if(C != UNCHANGED)
        SetC(C);
}

void SCC68070::SetXC(const uint8_t XC)
{
    SetX(XC);
    SetC(XC);
}

void SCC68070::SetVC(const uint8_t VC)
{
    SetV(VC);
    SetC(VC);
}

void SCC68070::SetX(const uint8_t X)
{
    SR &= 0b1111111111101111;
    SR |= (X << 4);
}

uint8_t SCC68070::GetX()
{
    return (SR & 0b0000000000010000) >> 4;
}

void SCC68070::SetN(const uint8_t N)
{
    SR &= 0b1111111111110111;
    SR |= (N << 3);
}

uint8_t SCC68070::GetN()
{
    return (SR & 0b0000000000001000) >> 3;
}

void SCC68070::SetZ(const uint8_t Z)
{
    SR &= 0b1111111111111011;
    SR |= (Z << 2);
}

uint8_t SCC68070::GetZ()
{
    return (SR & 0b0000000000000100) >> 2;
}

void SCC68070::SetV(const uint8_t V)
{
    SR &= 0b1111111111111101;
    SR |= (V << 1);
}

uint8_t SCC68070::GetV()
{
    return (SR & 0b0000000000000010) >> 1;
}

void SCC68070::SetC(const uint8_t C)
{
    SR &= 0b1111111111111110;
    SR |= C;
}

uint8_t SCC68070::GetC()
{
    return SR & 0b0000000000000001;
}

void SCC68070::SetS(const uint8_t S) // From Bizhawk
{
    if(S == GetS()) return;
    if(S) // entering supervisor mode
    {
        USP = A[7];
        A[7] = SSP;
        SR |= 0b0010000000000000;
    }
    else // exiting supervisor mode
    {
        SSP = A[7];
        A[7] = USP;
        SR &= 0b1101111111111111;
    }
}

uint8_t SCC68070::GetS()
{
    return (SR & 0b0010000000000000) >> 13;
}

void SCC68070::GenerateInstructionOpcodes(const char* format, std::vector<std::vector<int>> values, uint16_t (SCC68070::*instFunc)(), void (SCC68070::*disFunc)(uint32_t))
{
    if(values.size() == 1)
    {
        uint8_t pos = 0;
        uint8_t len = 0;
        for(int i = 0; i < 16; i++)
        {
            if(format[i] != '0' && format[i] != '1')
            {
                char c = format[i];
                pos = i;
                while(format[i] == c)
                {
                    len++;
                    i++;
                }
            }
        }

        for(int value : values[0])
        {
            std::string s(format, pos);
            s += toBinString(value, len);
            if(pos + len < 16)
                s += std::string(format + pos + len, 16 - pos - len);
            ILUT[binStringToInt(s)] = instFunc;
            DLUT[binStringToInt(s)] = disFunc;
        }
    }
    else
    {
        uint8_t pos = 0;
        uint8_t len = 0;
        for(int i = 0; i < 16; i++)
        {
            if(format[i] != '0' && format[i] != '1')
            {
                char c = format[i];
                pos = i;
                while(format[i] == c)
                {
                    len++;
                    i++;
                }
                break;
            }
        }

        for(int value : values[0])
        {
            std::string s(format, pos);
            s += toBinString(value, len);
            if(pos + len < 16)
                s += std::string(format + pos + len, 16 - pos - len);
            GenerateInstructionOpcodes(s.c_str(), std::vector<std::vector<int>>(values.begin()+1, values.end()), instFunc, disFunc);
        }
    }
}

void SCC68070::GenerateInstructionSet()
{
#define FULL_BYTE {\
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,\
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,\
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,\
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,\
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,\
    200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,\
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,\
}

    for(int i = 0; i <= UINT16_MAX; i++) // init LUT with unknown instructions
    {
        ILUT[i] = &SCC68070::UnknownInstruction;
        DLUT[i] = &SCC68070::DisassembleUnknownInstruction;
    }

    GenerateInstructionOpcodes("1100aaa10000bccc", {
        {0, 1, 2, 3, 4, 5, 6, 7}, // aaa
        {0, 1}, // b
        {0, 1, 2, 3, 4, 5, 6, 7}, // ccc
    }, &SCC68070::Abcd, &SCC68070::DisassembleAbcd);

    GenerateInstructionOpcodes("1101aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Add, &SCC68070::DisassembleAdd);
    GenerateInstructionOpcodes("1101aaabbb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Add, &SCC68070::DisassembleAdd);
    GenerateInstructionOpcodes("1101aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::Add, &SCC68070::DisassembleAdd);
    GenerateInstructionOpcodes("1101aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},       {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Add, &SCC68070::DisassembleAdd);
    GenerateInstructionOpcodes("1101aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},/*effective mode 7*/ {0, 1} }, &SCC68070::Add, &SCC68070::DisassembleAdd);

    GenerateInstructionOpcodes("1101aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Adda, &SCC68070::DisassembleAdda);
    GenerateInstructionOpcodes("1101aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::Adda, &SCC68070::DisassembleAdda);

    GenerateInstructionOpcodes("00000110aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6},  {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Addi, &SCC68070::DisassembleAddi);
    GenerateInstructionOpcodes("00000110aa111ccc", {{0, 1, 2}, /*effective mode 7*/ {0, 1} }, &SCC68070::Addi, &SCC68070::DisassembleAddi);

    GenerateInstructionOpcodes("0101aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Addq, &SCC68070::DisassembleAddq);
    GenerateInstructionOpcodes("0101aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1} }, &SCC68070::Addq, &SCC68070::DisassembleAddq);
    GenerateInstructionOpcodes("0101aaa0bb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Addq, &SCC68070::DisassembleAddq);

    GenerateInstructionOpcodes("1101aaa1bb00cddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Addx, &SCC68070::DisassembleAddx);

    GenerateInstructionOpcodes("1100aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::And, &SCC68070::DisassembleAnd);
    GenerateInstructionOpcodes("1100aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/* effective mode 7*/{0, 1, 2, 3, 4} }, &SCC68070::And, &SCC68070::DisassembleAnd);
    GenerateInstructionOpcodes("1100aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::And, &SCC68070::DisassembleAnd);
    GenerateInstructionOpcodes("1100aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},/* effective mode 7*/{0, 1} }, &SCC68070::And, &SCC68070::DisassembleAnd);

    GenerateInstructionOpcodes("00000010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Andi, &SCC68070::DisassembleAndi);
    GenerateInstructionOpcodes("00000010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::Andi, &SCC68070::DisassembleAndi);

    ILUT[0x023C] = &SCC68070::Andiccr;
    DLUT[0x023C] = &SCC68070::DisassembleAndiccr;

    ILUT[0x027C] = &SCC68070::Andisr;
    DLUT[0x027C] = &SCC68070::DisassembleAndisr;

    GenerateInstructionOpcodes("1110000a11bbbccc", {{0, 1}, {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::AsM, &SCC68070::DisassembleAsM);
    GenerateInstructionOpcodes("1110000a11111ccc", {{0, 1}, /* effective mode 7 */ {0, 1} }, &SCC68070::AsM, &SCC68070::DisassembleAsM);

    GenerateInstructionOpcodes("1110aaabccd00eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::AsR, &SCC68070::DisassembleAsR);

    GenerateInstructionOpcodes("0110aaaabbbbbbbb", {{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, FULL_BYTE }, &SCC68070::BCC, &SCC68070::DisassembleBCC);

    GenerateInstructionOpcodes("0000aaa101bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Bchg, &SCC68070::DisassembleBchg);
    GenerateInstructionOpcodes("0000aaa101111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} }, &SCC68070::Bchg, &SCC68070::DisassembleBchg);
    GenerateInstructionOpcodes("0000100001aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Bchg, &SCC68070::DisassembleBchg);
    ILUT[0x0878] = &SCC68070::Bchg; ILUT[0x0879] = &SCC68070::Bchg; /*effective mode 7*/
    DLUT[0x0878] = &SCC68070::DisassembleBchg;
    DLUT[0x0879] = &SCC68070::DisassembleBchg;

    GenerateInstructionOpcodes("0000aaa110bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Bclr, &SCC68070::DisassembleBclr);
    GenerateInstructionOpcodes("0000aaa110111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} }, &SCC68070::Bclr, &SCC68070::DisassembleBclr);
    GenerateInstructionOpcodes("0000100010aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Bclr, &SCC68070::DisassembleBclr);
    ILUT[0x08B8] = &SCC68070::Bclr; ILUT[0x08B9] = &SCC68070::Bclr; /*effective mode 7*/
    DLUT[0x08B8] = &SCC68070::DisassembleBclr;
    DLUT[0x08B9] = &SCC68070::DisassembleBclr;

    GenerateInstructionOpcodes("01100000aaaaaaaa", {FULL_BYTE }, &SCC68070::Bra, &SCC68070::DisassembleBra);

    GenerateInstructionOpcodes("0000aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Bset, &SCC68070::DisassembleBset);
    GenerateInstructionOpcodes("0000aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} }, &SCC68070::Bset, &SCC68070::DisassembleBset);
    GenerateInstructionOpcodes("0000100011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Bset, &SCC68070::DisassembleBset);
    ILUT[0x08F8] = &SCC68070::Bset; ILUT[0x08F9] = &SCC68070::Bset; /*effective mode 7*/
    DLUT[0x08F8] = &SCC68070::DisassembleBset;
    DLUT[0x08F9] = &SCC68070::DisassembleBset;

    GenerateInstructionOpcodes("01100001aaaaaaaa", {FULL_BYTE }, &SCC68070::Bsr, &SCC68070::DisassembleBsr);

    GenerateInstructionOpcodes("0000aaa100bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Btst, &SCC68070::DisassembleBtst);
    GenerateInstructionOpcodes("0000aaa100111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::Btst, &SCC68070::DisassembleBtst);
    GenerateInstructionOpcodes("0000100000aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Btst, &SCC68070::DisassembleBtst);
    ILUT[0x0838] = &SCC68070::Btst; ILUT[0x0839] = &SCC68070::Btst; ILUT[0x083A] = &SCC68070::Btst; ILUT[0x083B] = &SCC68070::Btst; /*effective mode 7*/
    DLUT[0x0838] = &SCC68070::DisassembleBtst;
    DLUT[0x0839] = &SCC68070::DisassembleBtst;
    DLUT[0x083A] = &SCC68070::DisassembleBtst;
    DLUT[0x083B] = &SCC68070::DisassembleBtst;

    GenerateInstructionOpcodes("0100aaa110bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Chk, &SCC68070::DisassembleChk);
    GenerateInstructionOpcodes("0100aaa110111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::Chk, &SCC68070::DisassembleChk);

    GenerateInstructionOpcodes("01000010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Clr, &SCC68070::DisassembleClr);
    GenerateInstructionOpcodes("01000010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::Clr, &SCC68070::DisassembleClr);

    GenerateInstructionOpcodes("1011aaa000cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, /* byte */ {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Cmp, &SCC68070::DisassembleCmp);
    GenerateInstructionOpcodes("1011aaa000111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, /* byte    effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::Cmp, &SCC68070::DisassembleCmp);
    GenerateInstructionOpcodes("1011aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Cmp, &SCC68070::DisassembleCmp);
    GenerateInstructionOpcodes("1011aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::Cmp, &SCC68070::DisassembleCmp);

    GenerateInstructionOpcodes("1011aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Cmpa, &SCC68070::DisassembleCmpa);
    GenerateInstructionOpcodes("1011aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::Cmpa, &SCC68070::DisassembleCmpa);

    GenerateInstructionOpcodes("00001100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Cmpi, &SCC68070::DisassembleCmpi);
    GenerateInstructionOpcodes("00001100aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::Cmpi, &SCC68070::DisassembleCmpi);

    GenerateInstructionOpcodes("1011aaa1bb001ccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Cmpm, &SCC68070::DisassembleCmpm);

    GenerateInstructionOpcodes("0101aaaa11001bbb", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::DbCC, &SCC68070::DisassembleDbCC);

    GenerateInstructionOpcodes("1000aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Divs, &SCC68070::DisassembleDivs);
    GenerateInstructionOpcodes("1000aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::Divs, &SCC68070::DisassembleDivs);

    GenerateInstructionOpcodes("1000aaa011bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Divu, &SCC68070::DisassembleDivu);
    GenerateInstructionOpcodes("1000aaa011111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::Divu, &SCC68070::DisassembleDivu);

    GenerateInstructionOpcodes("1011aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Eor, &SCC68070::DisassembleEor);
    GenerateInstructionOpcodes("1011aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::Eor, &SCC68070::DisassembleEor);

    GenerateInstructionOpcodes("00001010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Eori, &SCC68070::DisassembleEori);
    GenerateInstructionOpcodes("00001010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::Eori, &SCC68070::DisassembleEori);

    ILUT[0x0A3C] = &SCC68070::Eoriccr;
    DLUT[0x0A3C] = &SCC68070::DisassembleEoriccr;

    ILUT[0x0A7C] = &SCC68070::Eorisr;
    DLUT[0x0A7C] = &SCC68070::DisassembleEorisr;

    GenerateInstructionOpcodes("1100aaa1bbbbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0b01000, 0b01001, 0b10001}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Exg, &SCC68070::DisassembleExg);

    GenerateInstructionOpcodes("0100100aaa000bbb", {{0b010, 0b011, 0b111}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Ext, &SCC68070::DisassembleExt);

    ILUT[0x4AFC] = &SCC68070::Illegal;
    DLUT[0x4AFC] = &SCC68070::DisassembleIllegal;

    GenerateInstructionOpcodes("0100111011aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Jmp, &SCC68070::DisassembleJmp);
    GenerateInstructionOpcodes("0100111011111bbb", {/*mode 7*/ {0, 1, 2, 3} }, &SCC68070::Jmp, &SCC68070::DisassembleJmp);

    GenerateInstructionOpcodes("0100111010aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Jsr, &SCC68070::DisassembleJsr);
    GenerateInstructionOpcodes("0100111010111bbb", {/*mode 7*/ {0, 1, 2, 3} }, &SCC68070::Jsr, &SCC68070::DisassembleJsr);

    GenerateInstructionOpcodes("0100aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Lea, &SCC68070::DisassembleLea);
    GenerateInstructionOpcodes("0100aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/* mode 7*/ {0, 1, 2, 3} }, &SCC68070::Lea, &SCC68070::DisassembleLea);

    GenerateInstructionOpcodes("0100111001010aaa", {{0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Link, &SCC68070::DisassembleLink);

    GenerateInstructionOpcodes("1110001a11bbbccc", {{0, 1}, {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::LsM, &SCC68070::DisassembleLsM);
    GenerateInstructionOpcodes("1110001a11111ccc", {{0, 1}, {0, 1} }, &SCC68070::LsM, &SCC68070::DisassembleLsM);

    GenerateInstructionOpcodes("1110aaabccd01eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::LsR, &SCC68070::DisassembleLsR);

    GenerateInstructionOpcodes("00aabbbcccdddeee", {{1, 2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Move, &SCC68070::DisassembleMove);
    GenerateInstructionOpcodes("00aabbb111dddeee", {{1, 2, 3}, {0, 1}, /* effective mode 7 */ {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Move, &SCC68070::DisassembleMove);
    GenerateInstructionOpcodes("00aabbbccc111eee", {{1, 2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::Move, &SCC68070::DisassembleMove);
    GenerateInstructionOpcodes("00aabbb111111eee", {{1, 2, 3}, {0, 1}, /* effective mode 7  effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::Move, &SCC68070::DisassembleMove);
    GenerateInstructionOpcodes("00aabbbccc001eee", {{2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Move, &SCC68070::DisassembleMove);
    GenerateInstructionOpcodes("00aabbb111001eee", {{2, 3}, {0, 1}, /* effective mode 7 effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Move, &SCC68070::DisassembleMove);

    GenerateInstructionOpcodes("001abbb001cccddd", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Movea, &SCC68070::DisassembleMovea);
    GenerateInstructionOpcodes("001abbb001111ddd", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::Movea, &SCC68070::DisassembleMovea);

    GenerateInstructionOpcodes("0100010011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Moveccr, &SCC68070::DisassembleMoveccr);
    GenerateInstructionOpcodes("0100010011111bbb", {/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::Moveccr, &SCC68070::DisassembleMoveccr);

    GenerateInstructionOpcodes("0100000011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MoveFsr, &SCC68070::DisassembleMoveFsr);
    ILUT[0x40F8] = &SCC68070::MoveFsr; ILUT[0x40F9] = &SCC68070::MoveFsr;
    DLUT[0x40F8] = &SCC68070::DisassembleMoveFsr;
    DLUT[0x40F9] = &SCC68070::DisassembleMoveFsr;

    GenerateInstructionOpcodes("0100011011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Movesr, &SCC68070::DisassembleMovesr);
    GenerateInstructionOpcodes("0100011011111bbb", {/*effective mode 7*/{0, 1, 2, 3, 4} }, &SCC68070::Movesr, &SCC68070::DisassembleMovesr);

    GenerateInstructionOpcodes("010011100110abbb", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Moveusp, &SCC68070::DisassembleMoveusp);

    GenerateInstructionOpcodes("010010001bcccddd", {{0, 1}, {2, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Movem, &SCC68070::DisassembleMovem);
    GenerateInstructionOpcodes("010010001b111ddd", {{0, 1}, /* mode 7 */ {0, 1} }, &SCC68070::Movem, &SCC68070::DisassembleMovem);
    GenerateInstructionOpcodes("010011001bcccddd", {{0, 1}, {2, 3, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Movem, &SCC68070::DisassembleMovem);
    GenerateInstructionOpcodes("010011001b111ddd", {{0, 1}, /* mode 7 */ {0, 1, 2, 3} }, &SCC68070::Movem, &SCC68070::DisassembleMovem);

    GenerateInstructionOpcodes("0000aaabbb001ccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6, 7}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Movep, &SCC68070::DisassembleMovep);

    GenerateInstructionOpcodes("0111aaa0bbbbbbbb", {{0, 1, 2, 3, 4, 5, 6, 7}, FULL_BYTE }, &SCC68070::Moveq, &SCC68070::DisassembleMoveq);

    GenerateInstructionOpcodes("1100aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Muls, &SCC68070::DisassembleMuls);
    GenerateInstructionOpcodes("1100aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::Muls, &SCC68070::DisassembleMuls);

    GenerateInstructionOpcodes("1100aaa011bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Mulu, &SCC68070::DisassembleMulu);
    GenerateInstructionOpcodes("1100aaa011111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::Mulu, &SCC68070::DisassembleMulu);

    GenerateInstructionOpcodes("0100100000aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Nbcd, &SCC68070::DisassembleNbcd);
    ILUT[0x4838] = &SCC68070::Nbcd; ILUT[0x4839] = &SCC68070::Nbcd;
    DLUT[0x4838] = &SCC68070::DisassembleNbcd;
    DLUT[0x4839] = &SCC68070::DisassembleNbcd;

    GenerateInstructionOpcodes("01000100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Neg, &SCC68070::DisassembleNeg);
    GenerateInstructionOpcodes("01000100aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::Neg, &SCC68070::DisassembleNeg);

    GenerateInstructionOpcodes("01000000aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Negx, &SCC68070::DisassembleNegx);
    GenerateInstructionOpcodes("01000000aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::Negx, &SCC68070::DisassembleNegx);

    ILUT[0x4E71] = &SCC68070::Nop;
    DLUT[0x4E71] = &SCC68070::DisassembleNop;

    GenerateInstructionOpcodes("01000110aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Not, &SCC68070::DisassembleNot);
    GenerateInstructionOpcodes("01000110aa111ccc", {{0, 1, 2}, {0, 1} }, &SCC68070::Not, &SCC68070::DisassembleNot);

    GenerateInstructionOpcodes("1000aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Or, &SCC68070::DisassembleOr);
    GenerateInstructionOpcodes("1000aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::Or, &SCC68070::DisassembleOr);
    GenerateInstructionOpcodes("1000aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Or, &SCC68070::DisassembleOr);
    GenerateInstructionOpcodes("1000aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::Or, &SCC68070::DisassembleOr);

    GenerateInstructionOpcodes("00000000aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Ori, &SCC68070::DisassembleOri);
    GenerateInstructionOpcodes("00000000aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::Ori, &SCC68070::DisassembleOri);

    ILUT[0x003C] = &SCC68070::Oriccr;
    DLUT[0x003C] = &SCC68070::DisassembleOriccr;

    ILUT[0x007C] = &SCC68070::Orisr;
    DLUT[0x007C] = &SCC68070::DisassembleOrisr;

    GenerateInstructionOpcodes("0100100001aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Pea, &SCC68070::DisassemblePea);
    GenerateInstructionOpcodes("0100100001111bbb", {/*mode 7*/ {0, 1, 2, 3} }, &SCC68070::Pea, &SCC68070::DisassemblePea);

    ILUT[0x4E70] = &SCC68070::Reset;
    DLUT[0x4E70] = &SCC68070::DisassembleReset;

    GenerateInstructionOpcodes("1110011a11bbbccc", {{0, 1},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::RoM, &SCC68070::DisassembleRoM);
    GenerateInstructionOpcodes("1110011a11111ccc", {{0, 1},/*effective mode 7*/ {0, 1} }, &SCC68070::RoM, &SCC68070::DisassembleRoM);

    GenerateInstructionOpcodes("1110aaabccd11eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::RoR, &SCC68070::DisassembleRoR);

    GenerateInstructionOpcodes("1110010a11bbbccc", {{0, 1},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::RoxM, &SCC68070::DisassembleRoxM);
    GenerateInstructionOpcodes("1110010a11111ccc", {{0, 1},/*effective mode 7*/ {0, 1} }, &SCC68070::RoxM, &SCC68070::DisassembleRoxM);

    GenerateInstructionOpcodes("1110aaabccd10eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::RoxR, &SCC68070::DisassembleRoxR);

    ILUT[0x4E73] = &SCC68070::Rte;
    DLUT[0x4E73] = &SCC68070::DisassembleRte;

    ILUT[0x4E77] = &SCC68070::Rtr;
    DLUT[0x4E77] = &SCC68070::DisassembleRtr;

    ILUT[0x4E75] = &SCC68070::Rts;
    DLUT[0x4E75] = &SCC68070::DisassembleRts;

    GenerateInstructionOpcodes("1000aaa10000bccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Sbcd, &SCC68070::DisassembleSbcd);

    GenerateInstructionOpcodes("0101aaaa11bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SCC, &SCC68070::DisassembleSCC);
    GenerateInstructionOpcodes("0101aaaa11111ccc", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},/*effective mode 7*/ {0, 1} }, &SCC68070::SCC, &SCC68070::DisassembleSCC);

    ILUT[0x4E72] = &SCC68070::Stop;
    DLUT[0x4E72] = &SCC68070::DisassembleStop;

    GenerateInstructionOpcodes("1001aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Sub, &SCC68070::DisassembleSub);
    GenerateInstructionOpcodes("1001aaabbb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Sub, &SCC68070::DisassembleSub);
    GenerateInstructionOpcodes("1001aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::Sub, &SCC68070::DisassembleSub);
    GenerateInstructionOpcodes("1001aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Sub, &SCC68070::DisassembleSub);
    GenerateInstructionOpcodes("1001aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},/*effective mode 7*/ {0, 1} }, &SCC68070::Sub, &SCC68070::DisassembleSub);

    GenerateInstructionOpcodes("1001aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Suba, &SCC68070::DisassembleSuba);
    GenerateInstructionOpcodes("1001aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4} }, &SCC68070::Suba, &SCC68070::DisassembleSuba);

    GenerateInstructionOpcodes("00000100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Subi, &SCC68070::DisassembleSubi);
    GenerateInstructionOpcodes("00000100aa111ccc", {{0, 1, 2}, {0, 1} }, &SCC68070::Subi, &SCC68070::DisassembleSubi);

    GenerateInstructionOpcodes("0101aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Subq, &SCC68070::DisassembleSubq);
    GenerateInstructionOpcodes("0101aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1} }, &SCC68070::Subq, &SCC68070::DisassembleSubq);
    GenerateInstructionOpcodes("0101aaa1bb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Subq, &SCC68070::DisassembleSubq);

    GenerateInstructionOpcodes("1001aaa1bb00cddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Subx, &SCC68070::DisassembleSubx);

    GenerateInstructionOpcodes("0100100001000aaa", {{0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Swap, &SCC68070::DisassembleSwap);

    GenerateInstructionOpcodes("0100101011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Tas, &SCC68070::DisassembleTas);
    ILUT[0x4AF8] = &SCC68070::Tas; ILUT[0x4AF9] = &SCC68070::Tas;
    DLUT[0x4AF8] = &SCC68070::DisassembleTas;
    DLUT[0x4AF9] = &SCC68070::DisassembleTas;

    GenerateInstructionOpcodes("010011100100aaaa", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15} }, &SCC68070::Trap, &SCC68070::DisassembleTrap);

    ILUT[0x4E76] = &SCC68070::Trapv;
    DLUT[0x4E76] = &SCC68070::DisassembleTrapv;

    GenerateInstructionOpcodes("01001010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Tst, &SCC68070::DisassembleTst);
    GenerateInstructionOpcodes("01001010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::Tst, &SCC68070::DisassembleTst);

    GenerateInstructionOpcodes("0100111001011aaa", {{0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Unlk, &SCC68070::DisassembleUnlk);
#undef FULL_BYTE
}
