#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"
#include "../../OS9/SystemCalls.hpp"

std::string SCC68070::exceptionVectorToString(uint8_t vector)
{
    switch(vector)
    {
        case 0:  return "Reset:Initial SSP";
        case 1:  return "Reset:Initial PC";
        case 2:  return "Bus error";
        case 3:  return "Address error";
        case 4:  return "Illegal instruction";
        case 5:  return "Zero divide";
        case 6:  return "CHK instruction";
        case 7:  return "TRAPV instruction";
        case 8:  return "Privilege violation";
        case 9:  return "Trace";
        case 10: return "Line 1010 emulator";
        case 11: return "Line 1111 emulator";
        case 14: return "Format error";
        case 15: return "Uninitialized vector interrupt";
        case 24: return "Spurious interrupt";
        case 25: return "Level 1 interrupt autovector";
        case 26: return "Level 2 interrupt autovector";
        case 27: return "Level 3 interrupt autovector";
        case 28: return "Level 4 interrupt autovector";
        case 29: return "Level 5 interrupt autovector";
        case 30: return "Level 6 interrupt autovector";
        case 31: return "Level 7 interrupt autovector";
        case 32: return "TRAP 0 instruction";
        case 33: return "TRAP 1 instruction";
        case 34: return "TRAP 2 instruction";
        case 35: return "TRAP 3 instruction";
        case 36: return "TRAP 4 instruction";
        case 37: return "TRAP 5 instruction";
        case 38: return "TRAP 6 instruction";
        case 39: return "TRAP 7 instruction";
        case 40: return "TRAP 8 instruction";
        case 41: return "TRAP 9 instruction";
        case 42: return "TRAP 10 instruction";
        case 43: return "TRAP 11 instruction";
        case 44: return "TRAP 12 instruction";
        case 45: return "TRAP 13 instruction";
        case 46: return "TRAP 14 instruction";
        case 47: return "TRAP 15 instruction";
        case 57: return "Level 1 on-chip interrupt autovector";
        case 58: return "Level 2 on-chip interrupt autovector";
        case 59: return "Level 3 on-chip interrupt autovector";
        case 60: return "Level 4 on-chip interrupt autovector";
        case 61: return "Level 5 on-chip interrupt autovector";
        case 62: return "Level 6 on-chip interrupt autovector";
        case 63: return "Level 7 on-chip interrupt autovector";
        default:
            if(vector >= 64)
                return "User interrupt vector " + std::to_string(vector - 64);
            return "Unknown vector " + std::to_string(vector);
    }
}

std::string SCC68070::DisassembleUnknownInstruction(const uint32_t pc) const
{
    return "Unknown instruction 0x" + toHex(currentOpcode);
}

std::string SCC68070::DisassembleABCD(const uint32_t pc) const
{
    const uint8_t Rx = (currentOpcode & 0x0E00) >> 9;
    const uint8_t Ry = (currentOpcode & 0x0007);
    const uint8_t rm = currentOpcode & 0x0008;
    return "ABCD " + (rm ? "-(A" + std::to_string(Ry) + "), -(A" + std::to_string(Rx) + ")" : \
                                         "D" + std::to_string(Ry) + ", D" + std::to_string(Rx));
}

std::string SCC68070::DisassembleADD(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    uint8_t size;
    if(opmode == 0 || opmode == 4)
        size = 1;
    else if(opmode == 1 || opmode == 5)
        size = 2;
    else
        size = 4;

    return std::string("ADD") + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (opmode < 3 ? DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg) \
                                                                                               : "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size));
}

std::string SCC68070::DisassembleADDA(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
          uint8_t   size = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(size)
        size = 4;
    else
        size = 2;

    return std::string("ADDA") + (size == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", A" + std::to_string(reg);
}

std::string SCC68070::DisassembleADDI(const uint32_t pc) const
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string((int8_t)cdi.board->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string((int16_t)cdi.board->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string((int32_t)cdi.board->GetLong(pc+2, NoFlags));
    }

    return std::string("ADDI") + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc + (size == 4 ? 6 : 4), eamode, eareg, size);
}

std::string SCC68070::DisassembleADDQ(const uint32_t pc) const
{
          uint8_t   data = (currentOpcode & 0x0E00) >> 9;
                    data = data ? data : 8;
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(size == 0)
        size = 1;
    else if(size == 1)
        size = 2;
    else
        size = 4;

    return std::string("ADDQ") + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + std::to_string(data) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size);
}

std::string SCC68070::DisassembleADDX(const uint32_t pc) const
{
    const uint8_t   Rx = (currentOpcode & 0x0E00) >> 9;
    const uint8_t   Ry = (currentOpcode & 0x0007);
          uint8_t size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t   rm = (currentOpcode & 0x0008) >> 3;

    if(size == 0)
        size = 1;
    else if(size == 1)
        size = 2;
    else
        size = 4;

    return std::string("ADDX") + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (rm ? "-(A" + std::to_string(Ry) + "), -(A" + std::to_string(Rx) + ")" \
                                                                                        : "D" + std::to_string(Ry) + ", D" + std::to_string(Rx));
}

std::string SCC68070::DisassembleAND(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    uint8_t size;
    if(opmode == 0 || opmode == 4)
        size = 1;
    else if(opmode == 1 || opmode == 5)
        size = 2;
    else
        size = 4;

    return std::string("AND") + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (opmode < 3 ? DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg) \
                                                                                               : "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size));
}

std::string SCC68070::DisassembleANDI(const uint32_t pc) const
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string(cdi.board->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string(cdi.board->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string(cdi.board->GetLong(pc+2, NoFlags));
    }

    return std::string("ANDI") + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc + (size == 4 ? 6 : 4), eamode, eareg, size);
}

std::string SCC68070::DisassembleANDICCR(const uint32_t pc) const
{
    const uint8_t data = cdi.board->GetWord(pc+2, NoFlags) & 0x1F;
    return "ANDI #0x" + toHex(data) + ", CCR";
}

std::string SCC68070::DisassembleANDISR(const uint32_t pc) const
{
    const uint16_t data = cdi.board->GetWord(pc+2, NoFlags);
    return "ANDI #0x" + toHex(data) + ", SR";
}

std::string SCC68070::DisassembleASm(const uint32_t pc) const
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    if(dr)
        return "ASL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
    else
        return "ASR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
}

std::string SCC68070::DisassembleASr(const uint32_t pc) const
{
    const uint8_t count = (currentOpcode & 0x0E00) >> 9;
    const uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    const uint8_t   reg = (currentOpcode & 0x0007);

    std::string leftOperand;
    if(ir)
        leftOperand = "D" + std::to_string(count);
    else
        leftOperand = "#" + std::to_string(count);

    if(dr)
        return std::string("ASL") + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
    else
        return std::string("ASR") + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleBcc(const uint32_t pc) const
{
    const uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    int16_t disp = (int8_t)(currentOpcode & 0x00FF);

    if(disp == 0)
        disp = cdi.board->GetWord(pc+2, NoFlags);

    return "B" + DisassembleConditionalCode(condition) + " " + toHex(pc + 2 + disp);
}

std::string SCC68070::DisassembleBCHG(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(currentOpcode & 0x0100)
    {
        const uint8_t reg = (currentOpcode & 0x0E00) >> 9;
        data += "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, eamode ? 1 : 4);
    }
    else
    {
        const uint8_t bit = (cdi.board->GetWord(pc+2, NoFlags) & 0x00FF) % (eamode ? 8 : 32);
        data += "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, eamode ? 1 : 4);
    }

    return "BCHG " + data;
}

std::string SCC68070::DisassembleBCLR(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(currentOpcode & 0x0100)
    {
        const uint8_t reg = (currentOpcode & 0x0E00) >> 9;
        data += "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, eamode ? 1 : 4);
    }
    else
    {
        const uint8_t bit = (cdi.board->GetWord(pc+2, NoFlags) & 0x00FF) % (eamode ? 8 : 32);
        data += "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, eamode ? 1 : 4);
    }

    return "BCLR " + data;
}

std::string SCC68070::DisassembleBRA(const uint32_t pc) const
{
    int16_t disp = (int8_t)(currentOpcode & 0x00FF);

    if(disp == 0)
        disp = cdi.board->GetWord(pc+2, NoFlags);

    return "BRA " + toHex(pc + 2 + disp);
}

std::string SCC68070::DisassembleBSET(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(currentOpcode & 0x0100)
    {
        const uint8_t reg = (currentOpcode & 0x0E00) >> 9;
        data += "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, eamode ? 1 : 4);
    }
    else
    {
        const uint8_t bit = (cdi.board->GetWord(pc+2, NoFlags) & 0x00FF) % (eamode ? 8 : 32);
        data += "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, eamode ? 1 : 4);
    }

    return "BSET " + data;
}

std::string SCC68070::DisassembleBSR(const uint32_t pc) const
{
    int16_t disp = (int8_t)(currentOpcode & 0x00FF);

    if(disp == 0)
        disp = cdi.board->GetWord(pc+2, NoFlags);

    return "BSR " + toHex(pc + 2 + disp);
}

std::string SCC68070::DisassembleBTST(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(currentOpcode & 0x0100)
    {
        const uint8_t reg = (currentOpcode & 0x0E00) >> 9;
        data = "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, eamode ? 1 : 4);
    }
    else
    {
        const uint8_t bit = (cdi.board->GetWord(pc+2, NoFlags) & 0x00FF) % (eamode ? 8 : 32);
        data = "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, eamode ? 1 : 4);
    }

    return "BTST " + data;
}

std::string SCC68070::DisassembleCHK(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "CHK " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleCLR(const uint32_t pc) const
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(size == 0)
        size = 1;
    else if(size == 1)
        size = 2;
    else
        size = 4;

    return std::string("CLR") + (size == 1 ? ".B " : (size == 2 ? ".W " : ".L ")) + DisassembleAddressingMode(pc+2, eamode, eareg, size);
}

std::string SCC68070::DisassembleCMP(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
          uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(opmode == 0)
        opmode = 1;
    else if(opmode == 1)
        opmode = 2;
    else
        opmode = 4;

    return std::string("CMP") + (opmode == 1 ? ".B " : opmode == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, opmode) + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleCMPA(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
          uint8_t opmode = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(opmode)
        opmode = 4;
    else
        opmode = 2;

    return std::string("CMPA") + (opmode == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, opmode) + ", A" + std::to_string(reg);
}

std::string SCC68070::DisassembleCMPI(const uint32_t pc) const
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string((int8_t)(cdi.board->GetByte(pc+3, NoFlags)));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string((int16_t)(cdi.board->GetWord(pc+2, NoFlags)));
    }
    else
    {
        size = 4;
        data = std::to_string((int32_t)(cdi.board->GetLong(pc+2, NoFlags)));
    }

    return std::string("CMPI") + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc + (size == 4 ? 6 : 4), eamode, eareg, size);
}

std::string SCC68070::DisassembleCMPM(const uint32_t pc) const
{
          uint8_t size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t   Ax = (currentOpcode & 0x0E00) >> 9;
    const uint8_t   Ay = (currentOpcode & 0x0007);

    if(size == 0)
        size = 1;
    else if(size == 1)
        size = 2;
    else
        size = 4;

    return std::string("CMPM") + (size == 1 ? ".B (A" : size == 2 ? ".W (A" : ".L (A") + std::to_string(Ay) + ")+, (A" + std::to_string(Ax) + ")+";
}

std::string SCC68070::DisassembleDBcc(const uint32_t pc) const
{
    const uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    const uint8_t       reg = (currentOpcode & 0x0007);
    const int16_t      disp = cdi.board->GetWord(pc+2, NoFlags);
    return "DB" + DisassembleConditionalCode(condition) + " D" + std::to_string(reg) + ", " + toHex(pc + 2 + disp);
}

std::string SCC68070::DisassembleDIVS(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "DIVS.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleDIVU(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "DIVU.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleEOR(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
          uint8_t opmode = (currentOpcode & 0x001C) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(opmode == 4)
        opmode = 1;
    else if(opmode == 5)
        opmode = 2;
    else
        opmode = 4;

    return std::string("EOR") + (opmode == 1 ? ".B D" : opmode == 2 ? ".W D" : ".L D") + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, opmode);
}

std::string SCC68070::DisassembleEORI(const uint32_t pc) const
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string(cdi.board->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string(cdi.board->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string(cdi.board->GetLong(pc+2, NoFlags));
    }

    return std::string("EORI") + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc+(size == 4 ? 6 : 4), eamode, eareg, size);
}

std::string SCC68070::DisassembleEORICCR(const uint32_t pc) const
{
    const uint8_t data = cdi.board->GetWord(pc+2, NoFlags) & 0x1F;
    return "EORI #0x" + toHex(data) + ", CCR";
}

std::string SCC68070::DisassembleEORISR(const uint32_t pc) const
{
    const uint8_t data = cdi.board->GetWord(pc+2, NoFlags);
    return "EORI #0x" + toHex(data) + ", SR";
}

std::string SCC68070::DisassembleEXG(const uint32_t pc) const
{
    const uint8_t   Rx = (currentOpcode & 0x0E00) >> 9;
    const uint8_t mode = (currentOpcode & 0x00F8) >> 3;
    const uint8_t   Ry = (currentOpcode & 0x0007);

    std::string left, right;
    if(mode == 0x08)
    {
        left = "D" + std::to_string(Rx);
        right = ", D" + std::to_string(Ry);
    }
    else if(mode == 0x09)
    {
        left = "A" + std::to_string(Rx);
        right = ", A" + std::to_string(Ry);
    }
    else
    {
        left = "D" + std::to_string(Rx);
        right = ", A" + std::to_string(Ry);
    }

    return "EXG " + left + right;
}

std::string SCC68070::DisassembleEXT(const uint32_t pc) const
{
    const uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t    reg = (currentOpcode & 0x0007);
    return std::string("EXT.") + (opmode == 2 ? "W D" : "L D") + std::to_string(reg);
}

std::string SCC68070::DisassembleILLEGAL(const uint32_t pc) const
{
    return "ILLEGAL";
}

std::string SCC68070::DisassembleJMP(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "JMP " + DisassembleAddressingMode(pc+2, eamode, eareg, 0);
}

std::string SCC68070::DisassembleJSR(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "JSR " + DisassembleAddressingMode(pc+2, eamode, eareg, 0);
}

std::string SCC68070::DisassembleLEA(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "LEA " + DisassembleAddressingMode(pc+2, eamode, eareg, 4) + ", A" + std::to_string(reg);
}

std::string SCC68070::DisassembleLINK(const uint32_t pc) const
{
    const uint8_t reg = (currentOpcode & 0x0007);
    const int16_t disp = cdi.board->GetWord(pc+2, NoFlags);
    return "LINK A" + std::to_string(reg) + ", #" + std::to_string(disp);
}

std::string SCC68070::DisassembleLSm(const uint32_t pc) const
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(dr)
        return "LSL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
    else
        return "LSR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
}

std::string SCC68070::DisassembleLSr(const uint32_t pc) const
{
    const uint8_t count = (currentOpcode & 0x0E00) >> 9;
    const uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    const uint8_t   reg = (currentOpcode & 0x0007);

    std::string leftOperand;
    if(ir)
        leftOperand = "D" + std::to_string(count);
    else
        leftOperand = "#" + std::to_string(count);

    if(dr)
        return std::string("LSL") + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
    else
        return std::string("LSR") + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleMOVE(const uint32_t pc) const
{
    const uint8_t    size = (currentOpcode & 0x3000) >> 12;
    const uint8_t  dstreg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t dstmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t srcmode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  srcreg = (currentOpcode & 0x0007);

    std::string srcEA, dstEA;
    if(size == 1)
    {
        srcEA = DisassembleAddressingMode(pc+2, srcmode, srcreg, 1);
        dstEA = DisassembleAddressingMode(pc+(srcmode == 7 && srcreg == 1 ? 6 : (srcmode >= 5 ? 4 : 2)), dstmode, dstreg, 1);
    }
    else if(size == 3)
    {
        srcEA = DisassembleAddressingMode(pc+2, srcmode, srcreg, 2);
        dstEA = DisassembleAddressingMode(pc+(srcmode == 7 && srcreg == 1 ? 6 : (srcmode >= 5 ? 4 : 2)), dstmode, dstreg, 2);
    }
    else
    {
        srcEA = DisassembleAddressingMode(pc+2, srcmode, srcreg, 4);
        dstEA = DisassembleAddressingMode(pc+(srcmode == 7 && (srcreg == 1 || srcreg == 4) ? 6 : (srcmode >= 5 ? 4 : 2)), dstmode, dstreg, 4);
    }

    return std::string("MOVE.") + ((size == 1) ? "B " : (size == 3) ? "W " : "L ") + srcEA + ", " + dstEA;
}

std::string SCC68070::DisassembleMOVEA(const uint32_t pc) const
{
    const uint8_t   size = (currentOpcode & 0x3000) >> 12;
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return std::string("MOVEA") + (size == 3 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size == 3 ? 2 : 4, true) + ", A" + std::to_string(reg);
}

std::string SCC68070::DisassembleMOVECCR(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "MOVE " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", CCR";
}

std::string SCC68070::DisassembleMOVEfSR(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "MOVE SR, " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
}

std::string SCC68070::DisassembleMOVESR(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "MOVE " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", SR";
}

std::string SCC68070::DisassembleMOVEUSP(const uint32_t pc) const
{
    const uint8_t  dr = (currentOpcode & 0x0008) >> 3;
    const uint8_t reg = (currentOpcode & 0x0007);
    return std::string("MOVE ") + (dr ? "USP, A" + std::to_string(reg) : "A" + std::to_string(reg) + ", USP");
}

std::string SCC68070::DisassembleMOVEM(const uint32_t pc) const
{
    const uint8_t     dr = (currentOpcode & 0x0400) >> 10;
    const uint8_t   size = (currentOpcode & 0x0040) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    const uint16_t mask = cdi.board->GetWord(pc+2, NoFlags);

    std::string list;
    list = toBinString(mask >> 8, 8) + " " + toBinString(mask, 8);

    return std::string("MOVEM") + (size ? ".L " : ".W ") + (dr ? DisassembleAddressingMode(pc+4, eamode, eareg, size ? 4 : 2) + ", " + list : list + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, size ? 4 : 2));
}

std::string SCC68070::DisassembleMOVEP(const uint32_t pc) const
{
    const uint8_t datareg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t  opmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t addrreg = (currentOpcode & 0x0007);
    const int16_t    disp = cdi.board->GetWord(pc+2, NoFlags);

    std::string operands;
    if(opmode == 4)
    {
        operands = ".W (" + std::to_string(disp) + ", A" + std::to_string(addrreg) + "), D" + std::to_string(datareg);
    }
    else if(opmode == 5)
    {
        operands = ".L (" + std::to_string(disp) + ", A" + std::to_string(addrreg) + "), D" + std::to_string(datareg);
    }
    else if(opmode == 6)
    {
        operands = ".W D" + std::to_string(datareg) + ", (" + std::to_string(disp) + ", A" + std::to_string(addrreg) + ")";
    }
    else
    {
        operands = ".L D" + std::to_string(datareg) + ", (" + std::to_string(disp) + ", A" + std::to_string(addrreg) + ")";
    }

    return "MOVEP" + operands;
}

std::string SCC68070::DisassembleMOVEQ(const uint32_t pc) const
{
    const uint8_t reg = (currentOpcode & 0x0E00) >> 9;
    const int8_t data = (currentOpcode & 0x00FF);
    return "MOVEQ #" + std::to_string(data) + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleMULS(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "MULS.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleMULU(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "MULU.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleNBCD(const uint32_t pc) const
{
    const uint8_t mode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  reg = (currentOpcode & 0x0007);
    return "NBCD " + DisassembleAddressingMode(pc+2, mode, reg, 1);
}

std::string SCC68070::DisassembleNEG(const uint32_t pc) const
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(size == 0)
        size = 1;
    else if(size == 1)
        size = 2;
    else
        size = 4;

    return std::string("NEG") + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size);
}

std::string SCC68070::DisassembleNEGX(const uint32_t pc) const
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(size == 0)
        size = 1;
    else if(size == 1)
        size = 2;
    else
        size = 4;

    return std::string("NEGX") + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size);
}

std::string SCC68070::DisassembleNOP(const uint32_t pc) const
{
    return "NOP";
}

std::string SCC68070::DisassembleNOT(const uint32_t pc) const
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(size == 0)
        size = 1;
    else if(size == 1)
        size = 2;
    else
        size = 4;

    return std::string("NOT") + (size == 1 ? ".B " : (size == 2 ? ".W " : ".L ")) + DisassembleAddressingMode(pc+2, eamode, eareg, size);
}

std::string SCC68070::DisassembleOR(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    uint8_t size;
    if(opmode == 0 || opmode == 4)
        size = 1;
    else if(opmode == 1 || opmode == 5)
        size = 2;
    else
        size = 4;

    return std::string("OR") + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (opmode < 3 ? DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg) \
                                                                                              : "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size));
}

std::string SCC68070::DisassembleORI(const uint32_t pc) const
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string(cdi.board->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string(cdi.board->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string(cdi.board->GetLong(pc+2, NoFlags));
    }

    return std::string("ORI") + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc+(size == 4 ? 6 : 4), eamode, eareg, size);
}

std::string SCC68070::DisassembleORICCR(const uint32_t pc) const
{
    const uint8_t data = cdi.board->GetWord(pc+2, NoFlags) & 0x1F;
    return "ORI #0x" + toHex(data) + ", CCR";
}

std::string SCC68070::DisassembleORISR(const uint32_t pc) const
{
    const uint16_t data = cdi.board->GetWord(pc+2, NoFlags);
    return "ORI #0x" + toHex(data) + ", SR";
}

std::string SCC68070::DisassemblePEA(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "PEA " + DisassembleAddressingMode(pc+2, eamode, eareg, 4);
}

std::string SCC68070::DisassembleRESET(const uint32_t pc) const
{
    return "RESET";
}

std::string SCC68070::DisassembleROm(const uint32_t pc) const
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(dr)
        return "ROL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
    else
        return "ROR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
}

std::string SCC68070::DisassembleROr(const uint32_t pc) const
{
    const uint8_t count = (currentOpcode & 0x0E00) >> 9;
    const uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    const uint8_t   reg = (currentOpcode & 0x0007);

    std::string leftOperand;
    if(ir)
        leftOperand = "D" + std::to_string(count);
    else
        leftOperand = "#" + std::to_string(count);

    if(dr)
        return std::string("ROL") + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
    else
        return std::string("ROR") + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleROXm(const uint32_t pc) const
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(dr)
        return "ROXL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
    else
        return "ROXR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
}

std::string SCC68070::DisassembleROXr(const uint32_t pc) const
{
    const uint8_t count = (currentOpcode & 0x0E00) >> 9;
    const uint8_t    dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t  size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t    ir = (currentOpcode & 0x0020) >> 5;
    const uint8_t   reg = (currentOpcode & 0x0007);

    std::string leftOperand;
    if(ir)
        leftOperand = "D" + std::to_string(count);
    else
        leftOperand = "#" + std::to_string(count);

    if(dr)
        return std::string("ROXL") + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
    else
        return std::string("ROXR") + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleRTE(const uint32_t pc) const
{
    return "RTE";
}

std::string SCC68070::DisassembleRTR(const uint32_t pc) const
{
    return "RTR";
}

std::string SCC68070::DisassembleRTS(const uint32_t pc) const
{
    return "RTS";
}

std::string SCC68070::DisassembleSBCD(const uint32_t pc) const
{
    const uint8_t Ry = (currentOpcode & 0x0E00) >> 9;
    const uint8_t rm = (currentOpcode & 0x0008);
    const uint8_t Rx = (currentOpcode & 0x0007);
    return "SBCD " + (rm ? "-(A" + std::to_string(Rx) + "), -(A" + std::to_string(Ry) + ")" : \
                                         "D" + std::to_string(Rx) + ", D" + std::to_string(Ry));
}

std::string SCC68070::DisassembleScc(const uint32_t pc) const
{
    const uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    const uint8_t    eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t     eareg = (currentOpcode & 0x0007);
    return "S" + DisassembleConditionalCode(condition) + " " + DisassembleAddressingMode(pc+2, eamode, eareg, 1);
}

std::string SCC68070::DisassembleSTOP(const uint32_t pc) const
{
    const uint16_t data = cdi.board->GetWord(pc+2, NoFlags);
    return "STOP #0x" + toHex(data);
}

std::string SCC68070::DisassembleSUB(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
          uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    uint8_t size;
    if(opmode == 0 || opmode == 4)
        size = 1;
    else if(opmode == 1 || opmode == 5)
        size = 2;
    else
        size = 4;

    return std::string("SUB") + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (opmode < 3 ? (DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg)) \
                                                                                               : ("D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size)));
}

std::string SCC68070::DisassembleSUBA(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
          uint8_t opmode = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(opmode)
        opmode = 4;
    else
        opmode = 2;

    return std::string("SUBA") + (opmode == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, opmode) + ", A" + std::to_string(reg);
}

std::string SCC68070::DisassembleSUBI(const uint32_t pc) const
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string((int8_t)(cdi.board->GetByte(pc+3, NoFlags)));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string((int16_t)(cdi.board->GetWord(pc+2, NoFlags)));
    }
    else
    {
        size = 4;
        data = std::to_string((int32_t)(cdi.board->GetLong(pc+2, NoFlags)));
    }

    return std::string("SUBI") + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc+ (size == 4 ? 6 : 4), eamode, eareg, size);
}

std::string SCC68070::DisassembleSUBQ(const uint32_t pc) const
{
          uint8_t   data = (currentOpcode & 0x0E00) >> 9;
                    data = data ? data : 8;
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(size == 0)
        size = 1;
    else if(size == 1)
        size = 2;
    else
        size = 4;

    return std::string("SUBQ") + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + std::to_string(data) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size);
}

std::string SCC68070::DisassembleSUBX(const uint32_t pc) const
{
    const uint8_t   ry = (currentOpcode & 0x0E00) >> 9;
          uint8_t size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t   rm = (currentOpcode & 0x0008) >> 3;
    const uint8_t   rx = (currentOpcode & 0x0007);

    if(size == 0)
        size = 1;
    else if(size == 1)
        size = 2;
    else
        size = 4;

    return std::string("SUBX") + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (rm ? "-(A" + std::to_string(rx) + "), -(A" + std::to_string(ry) + ")" \
                                                                                        : "D" + std::to_string(rx) + ", D" + std::to_string(ry));
}

std::string SCC68070::DisassembleSWAP(const uint32_t pc) const
{
    return "SWAP D" + std::to_string(currentOpcode & 0x0007);
}

std::string SCC68070::DisassembleTAS(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return "TAS " + DisassembleAddressingMode(pc+2, eamode, eareg, 1);
}

std::string SCC68070::DisassembleTRAP(const uint32_t pc) const
{
    return "TRAP #" + std::to_string(currentOpcode & 0x000F);
}

std::string SCC68070::DisassembleTRAPV(const uint32_t pc) const
{
    return "TRAPV";
}

std::string SCC68070::DisassembleTST(const uint32_t pc) const
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(size == 0)
        size = 1;
    else if(size == 1)
        size = 2;
    else
        size = 4;

    return "TST " + DisassembleAddressingMode(pc+2, eamode, eareg, size);
}

std::string SCC68070::DisassembleUNLK(const uint32_t pc) const
{
    return "UNLK A" + std::to_string(currentOpcode & 0x0007);
}
