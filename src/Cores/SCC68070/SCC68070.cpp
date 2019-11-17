#include <cstring>

#include "SCC68070.hpp"

SCC68070::SCC68070(CeDImu* cedimu, VDSC* gpu, const uint32_t clockFrequency) : app(cedimu), vdsc(gpu), instructionsBuffer(), ILUT()
{
    disassemble = false;
    Execute = &SCC68070::Interpreter;
    internal = new uint8_t[0x80008080-INTERNAL];
    count = 0;
    clockPeriod = (1.0L / clockFrequency) * 1000000000; // Time used to execute a clock cycle
#ifdef DEBUG
    out.open("SCC68070.txt");
    instruction.open("instructions.txt");
#endif // DEBUG
    GenerateInstructionSet();
    RebootCore();

    internal[USR] = 0x07;
}

SCC68070::~SCC68070()
{
    delete[] internal;
}

void SCC68070::RebootCore()
{
    executionTime = 0;
    for(uint8_t i = 0; i < 8; i++)
    {
        D[i] = 0;
        A[i] = 0;
    }
    instructionsBuffer.clear();
    ResetOperation();
    instructionsBufferChanged = true;
    stop = false;
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
    (this->*Execute)(false);
}

void SCC68070::Run()
{
    (this->*Execute)(true);
}

uint16_t SCC68070::GetNextWord()
{
    uint16_t opcode = vdsc->GetWord(PC);
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
    }
    else // exiting supervisor mode
    {
        SSP = A[7];
        A[7] = USP;
    }
    SR &= 0b1101111111111111;
    SR |= (S << 13);
}

uint8_t SCC68070::GetS()
{
    return (SR & 0b0010000000000000) >> 13;
}

void SCC68070::GenerateInstructionOpcodes(SCC68070InstructionSet instruction, const char* format, std::vector<std::vector<int>> values)
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
            ILUT.emplace(binStringToInt(s), instruction);
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
            GenerateInstructionOpcodes(instruction, s.c_str(), std::vector<std::vector<int>>(values.begin()+1, values.end()));
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
    GenerateInstructionOpcodes(ABCD, "1100aaa10000bccc", {
        {0, 1, 2, 3, 4, 5, 6, 7}, // aaa
        {0, 1}, // b
        {0, 1, 2, 3, 4, 5, 6, 7}, // ccc
    });

    GenerateInstructionOpcodes(ADD,  "1101aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(ADD,  "1101aaabbb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(ADD,  "1101aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} });
    GenerateInstructionOpcodes(ADD,  "1101aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},       {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(ADD,  "1101aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},/*effective mode 7*/ {0, 1} });

    GenerateInstructionOpcodes(ADDA, "1101aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(ADDA, "1101aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, /* effective mode 7 */ {0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(ADDI, "00000110aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6},  {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(ADDI, "00000110aa111ccc", {{0, 1, 2}, /*effective mode 7*/ {0, 1} });

    GenerateInstructionOpcodes(ADDQ, "0101aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(ADDQ, "0101aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1} });
    GenerateInstructionOpcodes(ADDQ, "0101aaa0bb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(ADDX, "1101aaa1bb00cddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(AND,  "1100aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(AND,  "1100aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/* effective mode 7*/{0, 1, 2, 3, 4} });
    GenerateInstructionOpcodes(AND,  "1100aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(AND,  "1100aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},/* effective mode 7*/{0, 1} });

    GenerateInstructionOpcodes(ANDI, "00000010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(ANDI, "00000010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} });

    ILUT.emplace(0x023C, ANDICCR);

    ILUT.emplace(0x027C, ANDISR);

    GenerateInstructionOpcodes(ASm,  "1110000a11bbbccc", {{0, 1}, {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(ASm,  "1110000a11111ccc", {{0, 1}, /* effective mode 7 */ {0, 1} });

    GenerateInstructionOpcodes(ASr,  "1110aaabccd00eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(Bcc,  "0110aaaabbbbbbbb", {{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, FULL_BYTE });

    GenerateInstructionOpcodes(BCHG, "0000aaa101bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(BCHG, "0000aaa101111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} });
    GenerateInstructionOpcodes(BCHG, "0000100001aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    ILUT.emplace(0x0878, BCHG); ILUT.emplace(0x0879, BCHG); /*effective mode 7*/

    GenerateInstructionOpcodes(BCLR, "0000aaa110bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(BCLR, "0000aaa110111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} });
    GenerateInstructionOpcodes(BCLR, "0000100010aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    ILUT.emplace(0x08B8, BCLR); ILUT.emplace(0x08B9, BCLR); /*effective mode 7*/

    GenerateInstructionOpcodes(BRA,  "01100000aaaaaaaa", {FULL_BYTE });

    GenerateInstructionOpcodes(BSET, "0000aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(BSET, "0000aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} });
    GenerateInstructionOpcodes(BSET, "0000100011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    ILUT.emplace(0x08F8, BSET); ILUT.emplace(0x08F9, BSET); /*effective mode 7*/

    GenerateInstructionOpcodes(BSR,  "01100001aaaaaaaa", {FULL_BYTE });

    GenerateInstructionOpcodes(BTST, "0000aaa100bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(BTST, "0000aaa100111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} });
    GenerateInstructionOpcodes(BTST, "0000100000aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    ILUT.emplace(0x0838, BTST); ILUT.emplace(0x0839, BTST); ILUT.emplace(0x083A, BTST); ILUT.emplace(0x083B, BTST); /*effective mode 7*/

    GenerateInstructionOpcodes(CHK,  "0100aaa110bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(CHK,  "0100aaa110111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(CLR,  "01000010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(CLR,  "01000010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} });

    GenerateInstructionOpcodes(CMP,  "1011aaa000cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, /* byte */ {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(CMP,  "1011aaa000111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, /* byte    effective mode 7 */ {0, 1, 2, 3, 4} });
    GenerateInstructionOpcodes(CMP,  "1011aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(CMP,  "1011aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 7 */ {0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(CMPA, "1011aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(CMPA, "1011aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, /* effective mode 7 */ {0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(CMPI, "00001100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(CMPI, "00001100aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} });

    GenerateInstructionOpcodes(CMPM, "1011aaa1bb001ccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(DBcc, "0101aaaa11001bbb", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(DIVS, "1000aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(DIVS, "1000aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(DIVU, "1000aaa011bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(DIVU, "1000aaa011111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(EOR,  "1011aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(EOR,  "1011aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1} });

    GenerateInstructionOpcodes(EORI, "00001010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(EORI, "00001010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} });

    ILUT.emplace(0x0A3C, EORICCR);

    ILUT.emplace(0x0A7C, EORISR);

    GenerateInstructionOpcodes(EXG,  "1100aaa1bbbbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0b01000, 0b01001, 0b10001}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(EXT,  "0100100aaa000bbb", {{0b010, 0b011, 0b111}, {0, 1, 2, 3, 4, 5, 6, 7} });

    ILUT.emplace(0x4AFC, ILLEGAL);

    GenerateInstructionOpcodes(JMP,  "0100111011aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(JMP,  "0100111011111bbb", {/*mode 7*/ {0, 1, 2, 3} });

    GenerateInstructionOpcodes(JSR,  "0100111010aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(JSR,  "0100111010111bbb", {/*mode 7*/ {0, 1, 2, 3} });

    GenerateInstructionOpcodes(LEA,  "0100aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(LEA,  "0100aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/* mode 7*/ {0, 1, 2, 3} });

    GenerateInstructionOpcodes(LINK, "0100111001010aaa", {{0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(LSm,  "1110001a11bbbccc", {{0, 1}, {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(LSm,  "1110001a11111ccc", {{0, 1}, {0, 1} });

    GenerateInstructionOpcodes(LSr,  "1110aaabccd01eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(MOVE, "00aabbbcccdddeee", {{1, 2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(MOVE, "00aabbb111dddeee", {{1, 2, 3}, {0, 1}, /* effective mode 7 */ {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(MOVE, "00aabbbccc111eee", {{1, 2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6},/*effective mode 7*/ {0, 1, 2, 3, 4} });
    GenerateInstructionOpcodes(MOVE, "00aabbb111111eee", {{1, 2, 3}, {0, 1}, /* effective mode 7  effective mode 7 */ {0, 1, 2, 3, 4} });
    GenerateInstructionOpcodes(MOVE, "00aabbbccc001eee", {{2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(MOVE, "00aabbb111001eee", {{2, 3}, {0, 1}, /* effective mode 7 effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(MOVEA, "001abbb001cccddd", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(MOVEA, "001abbb001111ddd", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7}, /* effective mode 7 */ {0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(MOVECCR, "0100010011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(MOVECCR, "0100010011111bbb", {/*effective mode 7*/ {0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(MOVEfSR, "0100000011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    ILUT.emplace(0x40F8, MOVEfSR); ILUT.emplace(0x40F9, MOVEfSR);

    GenerateInstructionOpcodes(MOVESR, "0100011011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(MOVESR, "0100011011111bbb", {/*effective mode 7*/{0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(MOVEUSP, "010011100110abbb", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(MOVEM, "010010001bcccddd", {{0, 1}, {2, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(MOVEM, "010010001b111ddd", {{0, 1}, /* mode 7 */ {0, 1} });
    GenerateInstructionOpcodes(MOVEM, "010011001bcccddd", {{0, 1}, {2, 3, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(MOVEM, "010011001b111ddd", {{0, 1}, /* mode 7 */ {0, 1, 2, 3} });

    GenerateInstructionOpcodes(MOVEP, "0000aaabbb001ccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6, 7}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(MOVEQ, "0111aaa0bbbbbbbb", {{0, 1, 2, 3, 4, 5, 6, 7}, FULL_BYTE });

    GenerateInstructionOpcodes(MULS, "1100aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(MULS, "1100aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(MULU, "1100aaa011bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(MULU, "1100aaa011111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(NBCD, "0100100000aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    ILUT.emplace(0x4838, NBCD); ILUT.emplace(0x4839, NBCD);

    GenerateInstructionOpcodes(NEG,  "01000100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(NEG,  "01000100aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} });

    GenerateInstructionOpcodes(NEGX, "01000000aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(NEGX, "01000000aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} });

    ILUT.emplace(0x4E71, NOP);

    GenerateInstructionOpcodes(NOT,  "01000110aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(NOT,  "01000110aa111ccc", {{0, 1, 2}, {0, 1} });

    GenerateInstructionOpcodes(OR,   "1000aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(OR,   "1000aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} });
    GenerateInstructionOpcodes(OR,   "1000aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(OR,   "1000aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1} });

    GenerateInstructionOpcodes(ORI,  "00000000aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(ORI,  "00000000aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} });

    ILUT.emplace(0x003C, ORICCR);

    ILUT.emplace(0x007C, ORISR);

    GenerateInstructionOpcodes(PEA,  "0100100001aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(PEA,  "0100100001111bbb", {/*mode 7*/ {0, 1, 2, 3} });

    ILUT.emplace(0x4E70, RESET);

    GenerateInstructionOpcodes(ROm,  "1110011a11bbbccc", {{0, 1},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(ROm,  "1110011a11111ccc", {{0, 1},/*effective mode 7*/ {0, 1} });

    GenerateInstructionOpcodes(ROr,  "1110aaabccd11eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(ROXm,  "1110010a11bbbccc", {{0, 1},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(ROXm,  "1110010a11111ccc", {{0, 1},/*effective mode 7*/ {0, 1} });

    GenerateInstructionOpcodes(ROXr,  "1110aaabccd10eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} });

    ILUT.emplace(0x4E73, RTE);

    ILUT.emplace(0x4E77, RTR);

    ILUT.emplace(0x4E75, RTS);

    GenerateInstructionOpcodes(SBCD, "1000aaa10000bccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(Scc,  "0101aaaa11bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(Scc,  "0101aaaa11111ccc", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},/*effective mode 7*/ {0, 1} });

    ILUT.emplace(0x4E72, STOP);

    GenerateInstructionOpcodes(SUB,  "1001aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(SUB,  "1001aaabbb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(SUB,  "1001aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} });
    GenerateInstructionOpcodes(SUB,  "1001aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(SUB,  "1001aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},/*effective mode 7*/ {0, 1} });

    GenerateInstructionOpcodes(SUBA, "1001aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(SUBA, "1001aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4} });

    GenerateInstructionOpcodes(SUBI, "00000100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(SUBI, "00000100aa111ccc", {{0, 1, 2}, {0, 1} });

    GenerateInstructionOpcodes(SUBQ, "0101aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(SUBQ, "0101aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1} });
    GenerateInstructionOpcodes(SUBQ, "0101aaa1bb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(SUBX, "1001aaa1bb00cddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(SWAP, "0100100001000aaa", {{0, 1, 2, 3, 4, 5, 6, 7} });

    GenerateInstructionOpcodes(TAS,  "0100101011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    ILUT.emplace(0x4AF8, TAS); ILUT.emplace(0x4AF9, TAS);

    GenerateInstructionOpcodes(TRAP, "010011100100aaaa", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15} });

    ILUT.emplace(0x4E76, TRAPV);

    GenerateInstructionOpcodes(TST,  "01001010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} });
    GenerateInstructionOpcodes(TST,  "01001010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} });

    GenerateInstructionOpcodes(UNLK, "0100111001011aaa", {{0, 1, 2, 3, 4, 5, 6, 7} });

//    std::ofstream ilut("ILUT.txt");
//    for(auto inst : ILUT)
//    {
//        ilut << std::hex << inst.first << std::endl;
//    }
//    ilut.close();
#undef FULL_BYTE
}
