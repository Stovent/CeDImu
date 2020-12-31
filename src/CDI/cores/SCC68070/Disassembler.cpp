#include "SCC68070.hpp"
#include "../../boards/Board.hpp"
#include "../../common/utils.hpp"

std::string SCC68070::DisassembleException(const uint8_t vectorNumber)const
{
    switch(vectorNumber)
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
        default: return "Unknown exception";
    }
}

std::string SCC68070::DisassembleUnknownInstruction(const uint32_t pc) const
{
    return toHex(pc) + "\tUnknown instruction 0x" + toHex(currentOpcode);
}

std::string SCC68070::DisassembleABCD(const uint32_t pc) const
{
    const uint8_t Rx = (currentOpcode & 0x0E00) >> 9;
    const uint8_t Ry = (currentOpcode & 0x0007);
    const uint8_t rm = currentOpcode & 0x0008;
    return toHex(pc) + "\tABCD " + (rm ? "-(A" + std::to_string(Ry) + "), -(A" + std::to_string(Rx) + ")" : \
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

    return toHex(pc) + "\tADD" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (opmode < 3 ? DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg) \
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

    return toHex(pc) + "\tADDA" + (size == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", A" + std::to_string(reg);
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
        data = std::to_string((int8_t)board->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string((int16_t)board->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string((int32_t)board->GetLong(pc+2, NoFlags));
    }

    return toHex(pc) + "\tADDI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc + (size == 4 ? 6 : 4), eamode, eareg, size);
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

    return toHex(pc) + "\tADDQ" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + std::to_string(data) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size);
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

    return toHex(pc) + "\tADDX" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (rm ? "-(A" + std::to_string(Ry) + "), -(A" + std::to_string(Rx) + ")" \
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

    return toHex(pc) + "\tAND" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (opmode < 3 ? DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg) \
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
        data = std::to_string(board->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string(board->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string(board->GetLong(pc+2, NoFlags));
    }

    return toHex(pc) + "\tANDI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc + (size == 4 ? 6 : 4), eamode, eareg, size);
}

std::string SCC68070::DisassembleANDICCR(const uint32_t pc) const
{
    const uint8_t data = board->GetWord(pc+2, NoFlags) & 0x1F;
    return toHex(pc) + "\tANDI #0x" + toHex(data) + ", CCR";
}

std::string SCC68070::DisassembleANDISR(const uint32_t pc) const
{
    const uint16_t data = board->GetWord(pc+2, NoFlags);
    return toHex(pc) + "\tANDI #0x" + toHex(data) + ", SR";
}

std::string SCC68070::DisassembleASm(const uint32_t pc) const
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    if(dr)
        return toHex(pc) + "\tASL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
    else
        return toHex(pc) + "\tASR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
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
        return toHex(pc) + "\tASL" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
    else
        return toHex(pc) + "\tASR" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleBcc(const uint32_t pc) const
{
    const uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    int16_t disp = (int8_t)(currentOpcode & 0x00FF);

    if(disp == 0)
        disp = board->GetWord(pc+2, NoFlags);

    return toHex(pc) + "\tB" + DisassembleConditionalCode(condition) + " " + toHex(pc + 2 + disp);
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
        const uint8_t bit = (board->GetWord(pc+2, NoFlags) & 0x00FF) % (eamode ? 8 : 32);
        data += "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, eamode ? 1 : 4);
    }

    return toHex(pc) + "\tBCHG " + data;
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
        const uint8_t bit = (board->GetWord(pc+2, NoFlags) & 0x00FF) % (eamode ? 8 : 32);
        data += "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, eamode ? 1 : 4);
    }

    return toHex(pc) + "\tBCLR " + data;
}

std::string SCC68070::DisassembleBRA(const uint32_t pc) const
{
    int16_t disp = (int8_t)(currentOpcode & 0x00FF);

    if(disp == 0)
        disp = board->GetWord(pc+2, NoFlags);

    return toHex(pc) + "\tBRA " + toHex(pc + 2 + disp);
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
        const uint8_t bit = (board->GetWord(pc+2, NoFlags) & 0x00FF) % (eamode ? 8 : 32);
        data += "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, eamode ? 1 : 4);
    }

    return toHex(pc) + "\tBSET " + data;
}

std::string SCC68070::DisassembleBSR(const uint32_t pc) const
{
    int16_t disp = (int8_t)(currentOpcode & 0x00FF);

    if(disp == 0)
        disp = board->GetWord(pc+2, NoFlags);

    return toHex(pc) + "\tBSR " + toHex(pc + 2 + disp);
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
        const uint8_t bit = (board->GetWord(pc+2, NoFlags) & 0x00FF) % (eamode ? 8 : 32);
        data = "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, eamode ? 1 : 4);
    }

    return toHex(pc) + "\tBTST " + data;
}

std::string SCC68070::DisassembleCHK(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tCHK " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg);
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

    return toHex(pc) + "\tCLR" + (size == 1 ? ".B " : (size == 2 ? ".W " : ".L ")) + DisassembleAddressingMode(pc+2, eamode, eareg, size);
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

    return toHex(pc) + "\tCMP" + (opmode == 1 ? ".B " : opmode == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, opmode) + ", D" + std::to_string(reg);
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

    return toHex(pc) + "\tCMPA" + (opmode == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, opmode) + ", A" + std::to_string(reg);
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
        data = std::to_string((int8_t)(board->GetByte(pc+3, NoFlags)));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string((int16_t)(board->GetWord(pc+2, NoFlags)));
    }
    else
    {
        size = 4;
        data = std::to_string((int32_t)(board->GetLong(pc+2, NoFlags)));
    }

    return toHex(pc) + "\tCMPI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc + (size == 4 ? 6 : 4), eamode, eareg, size);
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

    return toHex(pc) + "\tCMPM" + (size == 1 ? ".B (A" : size == 2 ? ".W (A" : ".L (A") + std::to_string(Ay) + ")+, (A" + std::to_string(Ax) + ")+";
}

std::string SCC68070::DisassembleDBcc(const uint32_t pc) const
{
    const uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    const uint8_t       reg = (currentOpcode & 0x0007);
    const int16_t      disp = board->GetWord(pc+2, NoFlags);
    return toHex(pc) + "\tDB" + DisassembleConditionalCode(condition) + " D" + std::to_string(reg) + ", " + toHex(pc + 2 + disp);
}

std::string SCC68070::DisassembleDIVS(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tDIVS.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleDIVU(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tDIVU.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg);
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

    return toHex(pc) + "\tEOR" + (opmode == 1 ? ".B D" : opmode == 2 ? ".W D" : ".L D") + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, opmode);
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
        data = std::to_string(board->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string(board->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string(board->GetLong(pc+2, NoFlags));
    }

    return toHex(pc) + "\tEORI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc+(size == 4 ? 6 : 4), eamode, eareg, size);
}

std::string SCC68070::DisassembleEORICCR(const uint32_t pc) const
{
    const uint8_t data = board->GetWord(pc+2, NoFlags) & 0x1F;
    return toHex(pc) + "\tEORI #0x" + toHex(data) + ", CCR";
}

std::string SCC68070::DisassembleEORISR(const uint32_t pc) const
{
    const uint8_t data = board->GetWord(pc+2, NoFlags);
    return toHex(pc) + "\tEORI #0x" + toHex(data) + ", SR";
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

    return toHex(pc) + "\tEXG " + left + right;
}

std::string SCC68070::DisassembleEXT(const uint32_t pc) const
{
    const uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t    reg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tEXT." + (opmode == 2 ? "W D" : "L D") + std::to_string(reg);
}

std::string SCC68070::DisassembleILLEGAL(const uint32_t pc) const
{
    return toHex(pc) + "\tILLEGAL";
}

std::string SCC68070::DisassembleJMP(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tJMP " + DisassembleAddressingMode(pc+2, eamode, eareg, 0);
}

std::string SCC68070::DisassembleJSR(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tJSR " + DisassembleAddressingMode(pc+2, eamode, eareg, 0);
}

std::string SCC68070::DisassembleLEA(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tLEA " + DisassembleAddressingMode(pc+2, eamode, eareg, 4) + ", A" + std::to_string(reg);
}

std::string SCC68070::DisassembleLINK(const uint32_t pc) const
{
    const uint8_t reg = (currentOpcode & 0x0007);
    const int16_t disp = board->GetWord(pc+2, NoFlags);
    return (toHex(pc) + "\tLINK A" + std::to_string(reg) + ", #" + std::to_string(disp));
}

std::string SCC68070::DisassembleLSm(const uint32_t pc) const
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(dr)
        return toHex(pc) + "\tLSL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
    else
        return toHex(pc) + "\tLSR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
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
        return toHex(pc) + "\tLSL" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
    else
        return toHex(pc) + "\tLSR" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
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

    return toHex(pc) + "\tMOVE." + ((size == 1) ? "B " : (size == 3) ? "W " : "L ") + srcEA + ", " + dstEA;
}

std::string SCC68070::DisassembleMOVEA(const uint32_t pc) const
{
    const uint8_t   size = (currentOpcode & 0x3000) >> 12;
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tMOVEA" + (size == 3 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size == 3 ? 2 : 4, true) + ", A" + std::to_string(reg);
}

std::string SCC68070::DisassembleMOVECCR(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tMOVE " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", CCR";
}

std::string SCC68070::DisassembleMOVEfSR(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tMOVE SR, " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
}

std::string SCC68070::DisassembleMOVESR(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tMOVE " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", SR";
}

std::string SCC68070::DisassembleMOVEUSP(const uint32_t pc) const
{
    const uint8_t  dr = (currentOpcode & 0x0008) >> 3;
    const uint8_t reg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tMOVE " + (dr ? "USP, A" + std::to_string(reg) : "A" + std::to_string(reg) + ", USP");
}

std::string SCC68070::DisassembleMOVEM(const uint32_t pc) const
{
    const uint8_t     dr = (currentOpcode & 0x0400) >> 10;
    const uint8_t   size = (currentOpcode & 0x0040) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    const uint16_t mask = board->GetWord(pc+2, NoFlags);

    std::string list;
    list = toBinString(mask >> 8, 8) + " " + toBinString(mask, 8);

    return toHex(pc) + "\tMOVEM" + (size ? ".L " : ".W ") + (dr ? DisassembleAddressingMode(pc+4, eamode, eareg, size ? 4 : 2) + ", " + list : list + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, size ? 4 : 2));
}

std::string SCC68070::DisassembleMOVEP(const uint32_t pc) const
{
    const uint8_t datareg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t  opmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t addrreg = (currentOpcode & 0x0007);
    const int16_t    disp = board->GetWord(pc+2, NoFlags);

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

    return toHex(pc) + "\tMOVEP" + operands;
}

std::string SCC68070::DisassembleMOVEQ(const uint32_t pc) const
{
    const uint8_t reg = (currentOpcode & 0x0E00) >> 9;
    const int8_t data = (currentOpcode & 0x00FF);
    return toHex(pc) + "\tMOVEQ #" + std::to_string(data) + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleMULS(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tMULS.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleMULU(const uint32_t pc) const
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tMULU.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleNBCD(const uint32_t pc) const
{
    const uint8_t mode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  reg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tNBCD " + DisassembleAddressingMode(pc+2, mode, reg, 1);
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

    return toHex(pc) + "\tNEG" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size);
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

    return toHex(pc) + "\tNEGX" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size);
}

std::string SCC68070::DisassembleNOP(const uint32_t pc) const
{
    return toHex(pc) + "\tNOP";
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

    return toHex(pc) + "\tNOT" + (size == 1 ? ".B " : (size == 2 ? ".W " : ".L ")) + DisassembleAddressingMode(pc+2, eamode, eareg, size);
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

    return toHex(pc) + "\tOR" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (opmode < 3 ? DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg) \
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
        data = std::to_string(board->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string(board->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string(board->GetLong(pc+2, NoFlags));
    }

    return toHex(pc) + "\tORI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc+(size == 4 ? 6 : 4), eamode, eareg, size);
}

std::string SCC68070::DisassembleORICCR(const uint32_t pc) const
{
    const uint8_t data = board->GetWord(pc+2, NoFlags) & 0x1F;
    return toHex(pc) + "\tORI #0x" + toHex(data) + ", CCR";
}

std::string SCC68070::DisassembleORISR(const uint32_t pc) const
{
    const uint16_t data = board->GetWord(pc+2, NoFlags);
    return toHex(pc) + "\tORI #0x" + toHex(data) + ", SR";
}

std::string SCC68070::DisassemblePEA(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tPEA " + DisassembleAddressingMode(pc+2, eamode, eareg, 4);
}

std::string SCC68070::DisassembleRESET(const uint32_t pc) const
{
    return toHex(pc) + "\tRESET";
}

std::string SCC68070::DisassembleROm(const uint32_t pc) const
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(dr)
        return toHex(pc) + "\tROL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
    else
        return toHex(pc) + "\tROR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
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
        return toHex(pc) + "\tROL" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
    else
        return toHex(pc) + "\tROR" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleROXm(const uint32_t pc) const
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(dr)
        return toHex(pc) + "\tROXL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
    else
        return toHex(pc) + "\tROXR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2);
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
        return toHex(pc) + "\tROXL" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
    else
        return toHex(pc) + "\tROXR" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg);
}

std::string SCC68070::DisassembleRTE(const uint32_t pc) const
{
    return toHex(pc) + "\tRTE";
}

std::string SCC68070::DisassembleRTR(const uint32_t pc) const
{
    return toHex(pc) + "\tRTR";
}

std::string SCC68070::DisassembleRTS(const uint32_t pc) const
{
    return toHex(pc) + "\tRTS";
}

std::string SCC68070::DisassembleSBCD(const uint32_t pc) const
{
    const uint8_t Ry = (currentOpcode & 0x0E00) >> 9;
    const uint8_t rm = (currentOpcode & 0x0008);
    const uint8_t Rx = (currentOpcode & 0x0007);
    return toHex(pc) + "\tSBCD " + (rm ? "-(A" + std::to_string(Rx) + "), -(A" + std::to_string(Ry) + ")" : \
                                         "D" + std::to_string(Rx) + ", D" + std::to_string(Ry));
}

std::string SCC68070::DisassembleScc(const uint32_t pc) const
{
    const uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    const uint8_t    eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t     eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tS" + DisassembleConditionalCode(condition) + " " + DisassembleAddressingMode(pc+2, eamode, eareg, 1);
}

std::string SCC68070::DisassembleSTOP(const uint32_t pc) const
{
    const uint16_t data = board->GetWord(pc+2, NoFlags);
    return toHex(pc) + "\tSTOP #0x" + toHex(data);
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

    return toHex(pc) + "\tSUB" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (opmode < 3 ? (DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg)) \
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

    return toHex(pc) + "\tSUBA" + (opmode == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, opmode) + ", A" + std::to_string(reg);
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
        data = std::to_string((int8_t)(board->GetByte(pc+3, NoFlags)));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string((int16_t)(board->GetWord(pc+2, NoFlags)));
    }
    else
    {
        size = 4;
        data = std::to_string((int32_t)(board->GetLong(pc+2, NoFlags)));
    }

    return toHex(pc) + "\tSUBI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc+ (size == 4 ? 6 : 4), eamode, eareg, size);
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

    return toHex(pc) + "\tSUBQ" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + std::to_string(data) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size);
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

    return toHex(pc) + "\tSUBX" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (rm ? "-(A" + std::to_string(rx) + "), -(A" + std::to_string(ry) + ")" \
                                                                                        : "D" + std::to_string(rx) + ", D" + std::to_string(ry));
}

std::string SCC68070::DisassembleSWAP(const uint32_t pc) const
{
    return toHex(pc) + "\tSWAP D" + std::to_string(currentOpcode & 0x0007);
}

std::string SCC68070::DisassembleTAS(const uint32_t pc) const
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    return toHex(pc) + "\tTAS " + DisassembleAddressingMode(pc+2, eamode, eareg, 1);
}

std::string SCC68070::DisassembleTRAP(const uint32_t pc) const
{
    return toHex(pc) + "\tTRAP #" + std::to_string(currentOpcode & 0x000F);
}

std::string SCC68070::DisassembleTRAPV(const uint32_t pc) const
{
    return toHex(pc) + "\tTRAPV";
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

    return toHex(pc) + "\tTST " + DisassembleAddressingMode(pc+2, eamode, eareg, size);
}

std::string SCC68070::DisassembleUNLK(const uint32_t pc) const
{
    return toHex(pc) + "\tUNLK A" + std::to_string(currentOpcode & 0x0007);
}
