#include "SCC68070.hpp"

#include <cstring>

SCC68070::SCC68070(VDSC* gpu, const uint32_t clockFrequency) : disassembledInstructions(), ILUT()
{
    vdsc = gpu;
    disassemble = false;

    Execute = &SCC68070::Interpreter;
    internal = new uint8_t[SCC68070Peripherals::Size];
    cycleDelay = (1.0L / clockFrequency) * 1000000000;

    OPEN_LOG(out, "SCC68070.txt")
    OPEN_LOG(instruction, "instructions.txt")
    uart_out.open("uart_out", std::ios::binary | std::ios::out);
    uart_in.open("uart_in", std::ios::binary | std::ios::in);

    GenerateInstructionSet();
    Reset();

    SET_TX_READY
    SET_RX_READY
}

SCC68070::~SCC68070()
{
    delete[] internal;
}

void SCC68070::Run(const bool loop)
{
    (this->*Execute)(loop);
}

void SCC68070::Reset()
{
    run = false;
    LOG(out << "RESET" << std::endl; instruction << "RESET" << std::endl;)
    disassembledInstructions.clear();
    cycleCount = totalCycleCount = 146;

    lastAddress = 0;
    currentOpcode = 0;
    currentPC = 0;
    memset(internal, 0, SCC68070Peripherals::Size);

    for(uint8_t i = 0; i < 8; i++)
    {
        D[i] = 0;
        A[i] = 0;
    }
    ResetOperation();
}

void SCC68070::SetRegister(CPURegisters reg, const uint32_t value)
{
    switch(reg)
    {
    case CPURegisters::D0: D[0] = value; break;
    case CPURegisters::D1: D[1] = value; break;
    case CPURegisters::D2: D[2] = value; break;
    case CPURegisters::D3: D[3] = value; break;
    case CPURegisters::D4: D[4] = value; break;
    case CPURegisters::D5: D[5] = value; break;
    case CPURegisters::D6: D[6] = value; break;
    case CPURegisters::D7: D[7] = value; break;

    case CPURegisters::A0: A[0] = value; break;
    case CPURegisters::A1: A[1] = value; break;
    case CPURegisters::A2: A[2] = value; break;
    case CPURegisters::A3: A[3] = value; break;
    case CPURegisters::A4: A[4] = value; break;
    case CPURegisters::A5: A[5] = value; break;
    case CPURegisters::A6: A[6] = value; break;
    case CPURegisters::A7:
        A[7] = value;
        if(GetS())
            SSP = value;
        else
            USP = value;
        break;

    case CPURegisters::PC: PC = value; break;
    case CPURegisters::SR:
        if((SR & 0x2000) ^ (value & 0x2000))
            if(value & 0x2000)
            {
                USP = A[7];
                A[7] = SSP;
            }
            else
            {
                SSP = A[7];
                A[7] = USP;
            }
        SR = value;
        break;

    case CPURegisters::USP:
        USP = A[7];
        if(!GetS())
            A[7] = value;

    case CPURegisters::SSP:
        SSP = A[7];
        if(GetS())
            A[7] = value;
    }
}

std::map<std::string, uint32_t> SCC68070::GetRegisters()
{
    return {
        {"D0", D[0]},
        {"D1", D[1]},
        {"D2", D[2]},
        {"D3", D[3]},
        {"D4", D[4]},
        {"D5", D[5]},
        {"D6", D[6]},
        {"D7", D[7]},
        {"A0", A[0]},
        {"A1", A[1]},
        {"A2", A[2]},
        {"A3", A[3]},
        {"A4", A[4]},
        {"A5", A[5]},
        {"A6", A[6]},
        {"A7", A[7]},
        {"PC", PC},
        {"SR", SR},
        {"SSP", SSP},
        {"USP", USP},
    };
}

uint16_t SCC68070::GetNextWord(const uint8_t flags)
{
    uint16_t opcode = vdsc->GetWord(PC, flags);
    PC += 2;
    return opcode;
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

void SCC68070::SetXC(const bool XC)
{
    SetX(XC);
    SetC(XC);
}

void SCC68070::SetVC(const bool VC)
{
    SetV(VC);
    SetC(VC);
}

void SCC68070::SetX(const bool X)
{
    SR &= 0b1111111111101111;
    SR |= (X << 4);
}

bool SCC68070::GetX()
{
    return SR & 0b0000000000010000;
}

void SCC68070::SetN(const bool N)
{
    SR &= 0b1111111111110111;
    SR |= (N << 3);
}

bool SCC68070::GetN()
{
    return SR & 0b0000000000001000;
}

void SCC68070::SetZ(const bool Z)
{
    SR &= 0b1111111111111011;
    SR |= (Z << 2);
}

bool SCC68070::GetZ()
{
    return SR & 0b0000000000000100;
}

void SCC68070::SetV(const bool V)
{
    SR &= 0b1111111111111101;
    SR |= (V << 1);
}

bool SCC68070::GetV()
{
    return SR & 0b0000000000000010;
}

void SCC68070::SetC(const bool C)
{
    SR &= 0b1111111111111110;
    SR |= C;
}

bool SCC68070::GetC()
{
    return SR & 0b0000000000000001;
}

void SCC68070::SetS(const bool S) // From Bizhawk
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

bool SCC68070::GetS()
{
    return SR & 0b0010000000000000;
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
    }, &SCC68070::ABCD, &SCC68070::DisassembleABCD);

    GenerateInstructionOpcodes("1101aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADD, &SCC68070::DisassembleADD);
    GenerateInstructionOpcodes("1101aaabbb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADD, &SCC68070::DisassembleADD);
    GenerateInstructionOpcodes("1101aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::ADD, &SCC68070::DisassembleADD);
    GenerateInstructionOpcodes("1101aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},       {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADD, &SCC68070::DisassembleADD);
    GenerateInstructionOpcodes("1101aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},/*effective mode 7*/ {0, 1} }, &SCC68070::ADD, &SCC68070::DisassembleADD);

    GenerateInstructionOpcodes("1101aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADDA, &SCC68070::DisassembleADDA);
    GenerateInstructionOpcodes("1101aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::ADDA, &SCC68070::DisassembleADDA);

    GenerateInstructionOpcodes("00000110aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6},  {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADDI, &SCC68070::DisassembleADDI);
    GenerateInstructionOpcodes("00000110aa111ccc", {{0, 1, 2}, /*effective mode 7*/ {0, 1} }, &SCC68070::ADDI, &SCC68070::DisassembleADDI);

    GenerateInstructionOpcodes("0101aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADDQ, &SCC68070::DisassembleADDQ);
    GenerateInstructionOpcodes("0101aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1} }, &SCC68070::ADDQ, &SCC68070::DisassembleADDQ);
    GenerateInstructionOpcodes("0101aaa0bb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADDQ, &SCC68070::DisassembleADDQ);

    GenerateInstructionOpcodes("1101aaa1bb00cddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADDX, &SCC68070::DisassembleADDX);

    GenerateInstructionOpcodes("1100aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::AND, &SCC68070::DisassembleAND);
    GenerateInstructionOpcodes("1100aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/* effective mode 7*/{0, 1, 2, 3, 4} }, &SCC68070::AND, &SCC68070::DisassembleAND);
    GenerateInstructionOpcodes("1100aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::AND, &SCC68070::DisassembleAND);
    GenerateInstructionOpcodes("1100aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},/* effective mode 7*/{0, 1} }, &SCC68070::AND, &SCC68070::DisassembleAND);

    GenerateInstructionOpcodes("00000010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ANDI, &SCC68070::DisassembleANDI);
    GenerateInstructionOpcodes("00000010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::ANDI, &SCC68070::DisassembleANDI);

    ILUT[0x023C] = &SCC68070::ANDICCR;
    DLUT[0x023C] = &SCC68070::DisassembleANDICCR;

    ILUT[0x027C] = &SCC68070::ANDISR;
    DLUT[0x027C] = &SCC68070::DisassembleANDISR;

    GenerateInstructionOpcodes("1110000a11bbbccc", {{0, 1}, {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ASm, &SCC68070::DisassembleASm);
    GenerateInstructionOpcodes("1110000a11111ccc", {{0, 1}, /* effective mode 7 */ {0, 1} }, &SCC68070::ASm, &SCC68070::DisassembleASm);

    GenerateInstructionOpcodes("1110aaabccd00eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ASr, &SCC68070::DisassembleASr);

    GenerateInstructionOpcodes("0110aaaabbbbbbbb", {{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, FULL_BYTE }, &SCC68070::Bcc, &SCC68070::DisassembleBcc);

    GenerateInstructionOpcodes("0000aaa101bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BCHG, &SCC68070::DisassembleBCHG);
    GenerateInstructionOpcodes("0000aaa101111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} }, &SCC68070::BCHG, &SCC68070::DisassembleBCHG);
    GenerateInstructionOpcodes("0000100001aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BCHG, &SCC68070::DisassembleBCHG);
    ILUT[0x0878] = &SCC68070::BCHG; ILUT[0x0879] = &SCC68070::BCHG; /*effective mode 7*/
    DLUT[0x0878] = &SCC68070::DisassembleBCHG;
    DLUT[0x0879] = &SCC68070::DisassembleBCHG;

    GenerateInstructionOpcodes("0000aaa110bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BCLR, &SCC68070::DisassembleBCLR);
    GenerateInstructionOpcodes("0000aaa110111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} }, &SCC68070::BCLR, &SCC68070::DisassembleBCLR);
    GenerateInstructionOpcodes("0000100010aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BCLR, &SCC68070::DisassembleBCLR);
    ILUT[0x08B8] = &SCC68070::BCLR; ILUT[0x08B9] = &SCC68070::BCLR; /*effective mode 7*/
    DLUT[0x08B8] = &SCC68070::DisassembleBCLR;
    DLUT[0x08B9] = &SCC68070::DisassembleBCLR;

    GenerateInstructionOpcodes("01100000aaaaaaaa", {FULL_BYTE }, &SCC68070::BRA, &SCC68070::DisassembleBRA);

    GenerateInstructionOpcodes("0000aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BSET, &SCC68070::DisassembleBSET);
    GenerateInstructionOpcodes("0000aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} }, &SCC68070::BSET, &SCC68070::DisassembleBSET);
    GenerateInstructionOpcodes("0000100011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BSET, &SCC68070::DisassembleBSET);
    ILUT[0x08F8] = &SCC68070::BSET; ILUT[0x08F9] = &SCC68070::BSET; /*effective mode 7*/
    DLUT[0x08F8] = &SCC68070::DisassembleBSET;
    DLUT[0x08F9] = &SCC68070::DisassembleBSET;

    GenerateInstructionOpcodes("01100001aaaaaaaa", {FULL_BYTE }, &SCC68070::BSR, &SCC68070::DisassembleBSR);

    GenerateInstructionOpcodes("0000aaa100bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BTST, &SCC68070::DisassembleBTST);
    GenerateInstructionOpcodes("0000aaa100111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::BTST, &SCC68070::DisassembleBTST);
    GenerateInstructionOpcodes("0000100000aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BTST, &SCC68070::DisassembleBTST);
    ILUT[0x0838] = &SCC68070::BTST; ILUT[0x0839] = &SCC68070::BTST; ILUT[0x083A] = &SCC68070::BTST; ILUT[0x083B] = &SCC68070::BTST; /*effective mode 7*/
    DLUT[0x0838] = &SCC68070::DisassembleBTST;
    DLUT[0x0839] = &SCC68070::DisassembleBTST;
    DLUT[0x083A] = &SCC68070::DisassembleBTST;
    DLUT[0x083B] = &SCC68070::DisassembleBTST;

    GenerateInstructionOpcodes("0100aaa110bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CHK, &SCC68070::DisassembleCHK);
    GenerateInstructionOpcodes("0100aaa110111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::CHK, &SCC68070::DisassembleCHK);

    GenerateInstructionOpcodes("01000010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CLR, &SCC68070::DisassembleCLR);
    GenerateInstructionOpcodes("01000010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::CLR, &SCC68070::DisassembleCLR);

    GenerateInstructionOpcodes("1011aaa000cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, /* byte */ {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CMP, &SCC68070::DisassembleCMP);
    GenerateInstructionOpcodes("1011aaa000111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, /* byte    effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::CMP, &SCC68070::DisassembleCMP);
    GenerateInstructionOpcodes("1011aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CMP, &SCC68070::DisassembleCMP);
    GenerateInstructionOpcodes("1011aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::CMP, &SCC68070::DisassembleCMP);

    GenerateInstructionOpcodes("1011aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CMPA, &SCC68070::DisassembleCMPA);
    GenerateInstructionOpcodes("1011aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::CMPA, &SCC68070::DisassembleCMPA);

    GenerateInstructionOpcodes("00001100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CMPI, &SCC68070::DisassembleCMPI);
    GenerateInstructionOpcodes("00001100aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::CMPI, &SCC68070::DisassembleCMPI);

    GenerateInstructionOpcodes("1011aaa1bb001ccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CMPM, &SCC68070::DisassembleCMPM);

    GenerateInstructionOpcodes("0101aaaa11001bbb", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::DBcc, &SCC68070::DisassembleDBcc);

    GenerateInstructionOpcodes("1000aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::DIVS, &SCC68070::DisassembleDIVS);
    GenerateInstructionOpcodes("1000aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::DIVS, &SCC68070::DisassembleDIVS);

    GenerateInstructionOpcodes("1000aaa011bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::DIVU, &SCC68070::DisassembleDIVU);
    GenerateInstructionOpcodes("1000aaa011111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::DIVU, &SCC68070::DisassembleDIVU);

    GenerateInstructionOpcodes("1011aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::EOR, &SCC68070::DisassembleEOR);
    GenerateInstructionOpcodes("1011aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::EOR, &SCC68070::DisassembleEOR);

    GenerateInstructionOpcodes("00001010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::EORI, &SCC68070::DisassembleEORI);
    GenerateInstructionOpcodes("00001010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::EORI, &SCC68070::DisassembleEORI);

    ILUT[0x0A3C] = &SCC68070::EORICCR;
    DLUT[0x0A3C] = &SCC68070::DisassembleEORICCR;

    ILUT[0x0A7C] = &SCC68070::EORISR;
    DLUT[0x0A7C] = &SCC68070::DisassembleEORISR;

    GenerateInstructionOpcodes("1100aaa1bbbbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0b01000, 0b01001, 0b10001}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::EXG, &SCC68070::DisassembleEXG);

    GenerateInstructionOpcodes("0100100aaa000bbb", {{0b010, 0b011, 0b111}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::EXT, &SCC68070::DisassembleEXT);

    ILUT[0x4AFC] = &SCC68070::ILLEGAL;
    DLUT[0x4AFC] = &SCC68070::DisassembleILLEGAL;

    GenerateInstructionOpcodes("0100111011aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::JMP, &SCC68070::DisassembleJMP);
    GenerateInstructionOpcodes("0100111011111bbb", {/*mode 7*/ {0, 1, 2, 3} }, &SCC68070::JMP, &SCC68070::DisassembleJMP);

    GenerateInstructionOpcodes("0100111010aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::JSR, &SCC68070::DisassembleJSR);
    GenerateInstructionOpcodes("0100111010111bbb", {/*mode 7*/ {0, 1, 2, 3} }, &SCC68070::JSR, &SCC68070::DisassembleJSR);

    GenerateInstructionOpcodes("0100aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::LEA, &SCC68070::DisassembleLEA);
    GenerateInstructionOpcodes("0100aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/* mode 7*/ {0, 1, 2, 3} }, &SCC68070::LEA, &SCC68070::DisassembleLEA);

    GenerateInstructionOpcodes("0100111001010aaa", {{0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::LINK, &SCC68070::DisassembleLINK);

    GenerateInstructionOpcodes("1110001a11bbbccc", {{0, 1}, {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::LSm, &SCC68070::DisassembleLSm);
    GenerateInstructionOpcodes("1110001a11111ccc", {{0, 1}, {0, 1} }, &SCC68070::LSm, &SCC68070::DisassembleLSm);

    GenerateInstructionOpcodes("1110aaabccd01eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::LSr, &SCC68070::DisassembleLSr);

    GenerateInstructionOpcodes("00aabbbcccdddeee", {{1, 2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);
    GenerateInstructionOpcodes("00aabbb111dddeee", {{1, 2, 3}, {0, 1}, /* effective mode 7 */ {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);
    GenerateInstructionOpcodes("00aabbbccc111eee", {{1, 2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);
    GenerateInstructionOpcodes("00aabbb111111eee", {{1, 2, 3}, {0, 1}, /* effective mode 7  effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);
    GenerateInstructionOpcodes("00aabbbccc001eee", {{2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);
    GenerateInstructionOpcodes("00aabbb111001eee", {{2, 3}, {0, 1}, /* effective mode 7 effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);

    GenerateInstructionOpcodes("001abbb001cccddd", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEA, &SCC68070::DisassembleMOVEA);
    GenerateInstructionOpcodes("001abbb001111ddd", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::MOVEA, &SCC68070::DisassembleMOVEA);

    GenerateInstructionOpcodes("0100010011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVECCR, &SCC68070::DisassembleMOVECCR);
    GenerateInstructionOpcodes("0100010011111bbb", {/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::MOVECCR, &SCC68070::DisassembleMOVECCR);

    GenerateInstructionOpcodes("0100000011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEfSR, &SCC68070::DisassembleMOVEfSR);
    ILUT[0x40F8] = &SCC68070::MOVEfSR; ILUT[0x40F9] = &SCC68070::MOVEfSR;
    DLUT[0x40F8] = &SCC68070::DisassembleMOVEfSR;
    DLUT[0x40F9] = &SCC68070::DisassembleMOVEfSR;

    GenerateInstructionOpcodes("0100011011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVESR, &SCC68070::DisassembleMOVESR);
    GenerateInstructionOpcodes("0100011011111bbb", {/*effective mode 7*/{0, 1, 2, 3, 4} }, &SCC68070::MOVESR, &SCC68070::DisassembleMOVESR);

    GenerateInstructionOpcodes("010011100110abbb", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEUSP, &SCC68070::DisassembleMOVEUSP);

    GenerateInstructionOpcodes("010010001bcccddd", {{0, 1}, {2, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEM, &SCC68070::DisassembleMOVEM);
    GenerateInstructionOpcodes("010010001b111ddd", {{0, 1}, /* mode 7 */ {0, 1} }, &SCC68070::MOVEM, &SCC68070::DisassembleMOVEM);
    GenerateInstructionOpcodes("010011001bcccddd", {{0, 1}, {2, 3, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEM, &SCC68070::DisassembleMOVEM);
    GenerateInstructionOpcodes("010011001b111ddd", {{0, 1}, /* mode 7 */ {0, 1, 2, 3} }, &SCC68070::MOVEM, &SCC68070::DisassembleMOVEM);

    GenerateInstructionOpcodes("0000aaabbb001ccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6, 7}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEP, &SCC68070::DisassembleMOVEP);

    GenerateInstructionOpcodes("0111aaa0bbbbbbbb", {{0, 1, 2, 3, 4, 5, 6, 7}, FULL_BYTE }, &SCC68070::MOVEQ, &SCC68070::DisassembleMOVEQ);

    GenerateInstructionOpcodes("1100aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MULS, &SCC68070::DisassembleMULS);
    GenerateInstructionOpcodes("1100aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::MULS, &SCC68070::DisassembleMULS);

    GenerateInstructionOpcodes("1100aaa011bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MULU, &SCC68070::DisassembleMULU);
    GenerateInstructionOpcodes("1100aaa011111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::MULU, &SCC68070::DisassembleMULU);

    GenerateInstructionOpcodes("0100100000aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::NBCD, &SCC68070::DisassembleNBCD);
    ILUT[0x4838] = &SCC68070::NBCD; ILUT[0x4839] = &SCC68070::NBCD;
    DLUT[0x4838] = &SCC68070::DisassembleNBCD;
    DLUT[0x4839] = &SCC68070::DisassembleNBCD;

    GenerateInstructionOpcodes("01000100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::NEG, &SCC68070::DisassembleNEG);
    GenerateInstructionOpcodes("01000100aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::NEG, &SCC68070::DisassembleNEG);

    GenerateInstructionOpcodes("01000000aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::NEGX, &SCC68070::DisassembleNEGX);
    GenerateInstructionOpcodes("01000000aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::NEGX, &SCC68070::DisassembleNEGX);

    ILUT[0x4E71] = &SCC68070::NOP;
    DLUT[0x4E71] = &SCC68070::DisassembleNOP;

    GenerateInstructionOpcodes("01000110aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::NOT, &SCC68070::DisassembleNOT);
    GenerateInstructionOpcodes("01000110aa111ccc", {{0, 1, 2}, {0, 1} }, &SCC68070::NOT, &SCC68070::DisassembleNOT);

    GenerateInstructionOpcodes("1000aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::OR, &SCC68070::DisassembleOR);
    GenerateInstructionOpcodes("1000aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::OR, &SCC68070::DisassembleOR);
    GenerateInstructionOpcodes("1000aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::OR, &SCC68070::DisassembleOR);
    GenerateInstructionOpcodes("1000aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::OR, &SCC68070::DisassembleOR);

    GenerateInstructionOpcodes("00000000aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ORI, &SCC68070::DisassembleORI);
    GenerateInstructionOpcodes("00000000aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::ORI, &SCC68070::DisassembleORI);

    ILUT[0x003C] = &SCC68070::ORICCR;
    DLUT[0x003C] = &SCC68070::DisassembleORICCR;

    ILUT[0x007C] = &SCC68070::ORISR;
    DLUT[0x007C] = &SCC68070::DisassembleORISR;

    GenerateInstructionOpcodes("0100100001aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::PEA, &SCC68070::DisassemblePEA);
    GenerateInstructionOpcodes("0100100001111bbb", {/*mode 7*/ {0, 1, 2, 3} }, &SCC68070::PEA, &SCC68070::DisassemblePEA);

    ILUT[0x4E70] = &SCC68070::RESET;
    DLUT[0x4E70] = &SCC68070::DisassembleRESET;

    GenerateInstructionOpcodes("1110011a11bbbccc", {{0, 1},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ROm, &SCC68070::DisassembleROm);
    GenerateInstructionOpcodes("1110011a11111ccc", {{0, 1},/*effective mode 7*/ {0, 1} }, &SCC68070::ROm, &SCC68070::DisassembleROm);

    GenerateInstructionOpcodes("1110aaabccd11eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ROr, &SCC68070::DisassembleROr);

    GenerateInstructionOpcodes("1110010a11bbbccc", {{0, 1},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ROm, &SCC68070::DisassembleROm);
    GenerateInstructionOpcodes("1110010a11111ccc", {{0, 1},/*effective mode 7*/ {0, 1} }, &SCC68070::ROm, &SCC68070::DisassembleROm);

    GenerateInstructionOpcodes("1110aaabccd10eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ROr, &SCC68070::DisassembleROr);

    ILUT[0x4E73] = &SCC68070::RTE;
    DLUT[0x4E73] = &SCC68070::DisassembleRTE;

    ILUT[0x4E77] = &SCC68070::RTR;
    DLUT[0x4E77] = &SCC68070::DisassembleRTR;

    ILUT[0x4E75] = &SCC68070::RTS;
    DLUT[0x4E75] = &SCC68070::DisassembleRTS;

    GenerateInstructionOpcodes("1000aaa10000bccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SBCD, &SCC68070::DisassembleSBCD);

    GenerateInstructionOpcodes("0101aaaa11bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Scc, &SCC68070::DisassembleScc);
    GenerateInstructionOpcodes("0101aaaa11111ccc", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},/*effective mode 7*/ {0, 1} }, &SCC68070::Scc, &SCC68070::DisassembleScc);

    ILUT[0x4E72] = &SCC68070::STOP;
    DLUT[0x4E72] = &SCC68070::DisassembleSTOP;

    GenerateInstructionOpcodes("1001aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUB, &SCC68070::DisassembleSUB);
    GenerateInstructionOpcodes("1001aaabbb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUB, &SCC68070::DisassembleSUB);
    GenerateInstructionOpcodes("1001aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::SUB, &SCC68070::DisassembleSUB);
    GenerateInstructionOpcodes("1001aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUB, &SCC68070::DisassembleSUB);
    GenerateInstructionOpcodes("1001aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},/*effective mode 7*/ {0, 1} }, &SCC68070::SUB, &SCC68070::DisassembleSUB);

    GenerateInstructionOpcodes("1001aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUBA, &SCC68070::DisassembleSUBA);
    GenerateInstructionOpcodes("1001aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4} }, &SCC68070::SUBA, &SCC68070::DisassembleSUBA);

    GenerateInstructionOpcodes("00000100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUBI, &SCC68070::DisassembleSUBI);
    GenerateInstructionOpcodes("00000100aa111ccc", {{0, 1, 2}, {0, 1} }, &SCC68070::SUBI, &SCC68070::DisassembleSUBI);

    GenerateInstructionOpcodes("0101aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUBQ, &SCC68070::DisassembleSUBQ);
    GenerateInstructionOpcodes("0101aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1} }, &SCC68070::SUBQ, &SCC68070::DisassembleSUBQ);
    GenerateInstructionOpcodes("0101aaa1bb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUBQ, &SCC68070::DisassembleSUBQ);

    GenerateInstructionOpcodes("1001aaa1bb00cddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUBX, &SCC68070::DisassembleSUBX);

    GenerateInstructionOpcodes("0100100001000aaa", {{0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SWAP, &SCC68070::DisassembleSWAP);

    GenerateInstructionOpcodes("0100101011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::TAS, &SCC68070::DisassembleTAS);
    ILUT[0x4AF8] = &SCC68070::TAS; ILUT[0x4AF9] = &SCC68070::TAS;
    DLUT[0x4AF8] = &SCC68070::DisassembleTAS;
    DLUT[0x4AF9] = &SCC68070::DisassembleTAS;

    GenerateInstructionOpcodes("010011100100aaaa", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15} }, &SCC68070::TRAP, &SCC68070::DisassembleTRAP);

    ILUT[0x4E76] = &SCC68070::TRAPV;
    DLUT[0x4E76] = &SCC68070::DisassembleTRAPV;

    GenerateInstructionOpcodes("01001010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::TST, &SCC68070::DisassembleTST);
    GenerateInstructionOpcodes("01001010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::TST, &SCC68070::DisassembleTST);

    GenerateInstructionOpcodes("0100111001011aaa", {{0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::UNLK, &SCC68070::DisassembleUNLK);
#undef FULL_BYTE
}
