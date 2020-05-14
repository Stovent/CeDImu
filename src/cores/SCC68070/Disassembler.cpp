#include "SCC68070.hpp"

#include "../../utils.hpp"

std::string SCC68070::DisassembleException(const uint8_t vectorNumber)
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
        case 32: return "TRAP 1 instruction";
        case 33: return "TRAP 2 instruction";
        case 34: return "TRAP 3 instruction";
        case 35: return "TRAP 4 instruction";
        case 36: return "TRAP 5 instruction";
        case 37: return "TRAP 6 instruction";
        case 38: return "TRAP 7 instruction";
        case 39: return "TRAP 8 instruction";
        case 40: return "TRAP 9 instruction";
        case 41: return "TRAP 10 instruction";
        case 42: return "TRAP 11 instruction";
        case 43: return "TRAP 12 instruction";
        case 44: return "TRAP 13 instruction";
        case 45: return "TRAP 14 instruction";
        case 46: return "TRAP 15 instruction";
        case 47: return "TRAP 16 instruction";
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

void SCC68070::DisassembleUnknownInstruction(uint32_t pc)
{
    disassembledInstructions.push_back(toHex(pc) + "\tUnknown instruction 0x" + toHex(currentOpcode));
}

void SCC68070::DisassembleAbcd(uint32_t pc)
{
    const uint8_t Rx = (currentOpcode & 0x0E00) >> 9;
    const uint8_t Ry = (currentOpcode & 0x0007);
    const uint8_t rm = currentOpcode & 0x0008;
    disassembledInstructions.push_back(toHex(pc) + "\tABCD " + (rm ? "-(A" + std::to_string(Ry) + "), -(A" + std::to_string(Rx) + ")" : \
                                                                 "D" + std::to_string(Ry) + ", D" + std::to_string(Rx)));
}

void SCC68070::DisassembleAdd(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tADD" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") \
                                                     + (opmode < 3 ? DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg) : \
                                                                     "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size)));
}

void SCC68070::DisassembleAdda(uint32_t pc)
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
          uint8_t   size = (currentOpcode & 0x0100) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(size)
        size = 4;
    else
        size = 2;

    disassembledInstructions.push_back(toHex(pc) + "\tADDA" + (size == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", A" + std::to_string(reg));
}

void SCC68070::DisassembleAddi(uint32_t pc)
{
          uint8_t   size = (currentOpcode & 0b0000000011000000) >> 6;
    const uint8_t eamode = (currentOpcode & 0b0000000000111000) >> 3;
    const uint8_t  eareg = (currentOpcode & 0b0000000000000111);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string((int8_t)vdsc->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string((int16_t)vdsc->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string((int32_t)vdsc->GetLong(pc+2, NoFlags));
    }

    disassembledInstructions.push_back(toHex(pc) + "\tADDI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc + (size == 4 ? size : 2), eamode, eareg, size));
}

void SCC68070::DisassembleAddq(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tADDQ" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + std::to_string(data) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size));
}

void SCC68070::DisassembleAddx(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tADDX" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (rm ? "-(A" + std::to_string(Ry) + "), -(A" + std::to_string(Rx) + ")" : \
                                                                                                                "D" + std::to_string(Ry) + ", D" + std::to_string(Rx)));
}

void SCC68070::DisassembleAnd(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tAND" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (opmode < 3 ? DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg) : \
                                                                                                                       "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size)));
}

void SCC68070::DisassembleAndi(uint32_t pc)
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string((int8_t)vdsc->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string((int16_t)vdsc->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string((int32_t)vdsc->GetLong(pc+2, NoFlags));
    }

    disassembledInstructions.push_back(toHex(pc) + "\tANDI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc + (size == 4 ? size : 2), eamode, eareg, size));
}

void SCC68070::DisassembleAndiccr(uint32_t pc)
{
    const uint8_t data = vdsc->GetWord(pc+2, NoFlags) & 0x1F;
    disassembledInstructions.push_back(toHex(pc) + "\tANDI #0x" + toHex(data) + ", CCR");
}

void SCC68070::DisassembleAndisr(uint32_t pc)
{
    const uint16_t data = vdsc->GetWord(pc+2, NoFlags);
    disassembledInstructions.push_back(toHex(pc) + "\tANDI #0x" + toHex(data) + ", SR");
}

void SCC68070::DisassembleAsM(uint32_t pc)
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    if(dr)
        disassembledInstructions.push_back(toHex(pc) + "\tASL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2));
    else
        disassembledInstructions.push_back(toHex(pc) + "\tASR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2));
}

void SCC68070::DisassembleAsR(uint32_t pc)
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
        disassembledInstructions.push_back(toHex(pc) + "\tASL" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg));
    else
        disassembledInstructions.push_back(toHex(pc) + "\tASR" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg));
}

void SCC68070::DisassembleBCC(uint32_t pc)
{
    const uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    int16_t disp = (int8_t)(currentOpcode & 0x00FF);

    if(disp == 0)
        disp = vdsc->GetWord(pc+2, NoFlags);

    disassembledInstructions.push_back(toHex(pc) + "\tB" + DisassembleConditionalCode(condition) + " " + toHex(pc + 2 + disp));
}

void SCC68070::DisassembleBchg(uint32_t pc)
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(currentOpcode & 0x0100)
    {
        const uint8_t reg = (currentOpcode & 0x0E00) >> 9;
        data += "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, 1);
    }
    else
    {
        const uint8_t bit = (vdsc->GetWord(pc+2, NoFlags) & 0x00FF) % 8;
        data += "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, 4);
    }

    disassembledInstructions.push_back(toHex(pc) + "\tBCHG " + data);
}

void SCC68070::DisassembleBclr(uint32_t pc)
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(currentOpcode & 0x0100)
    {
        const uint8_t reg = (currentOpcode & 0x0E00) >> 9;
        data += "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, 1);
    }
    else
    {
        const uint8_t bit = (vdsc->GetWord(pc+2, NoFlags) & 0x00FF) % 8;
        data += "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, 4);
    }

    disassembledInstructions.push_back(toHex(pc) + "\tBCLR " + data);
}

void SCC68070::DisassembleBra(uint32_t pc)
{
    int16_t disp = (int8_t)(currentOpcode & 0x00FF);

    if(disp == 0)
        disp = vdsc->GetWord(pc+2, NoFlags);

    disassembledInstructions.push_back(toHex(pc) + "\tBRA " + toHex(pc + 2 + disp));
}

void SCC68070::DisassembleBset(uint32_t pc)
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(currentOpcode & 0x0100)
    {
        const uint8_t reg = (currentOpcode & 0x0E00) >> 9;
        data += "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, 1);
    }
    else
    {
        const uint8_t bit = (vdsc->GetWord(pc+2, NoFlags) & 0x00FF) % 8;
        data += "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, 4);
    }

    disassembledInstructions.push_back(toHex(pc) + "\tBSET " + data);
}

void SCC68070::DisassembleBsr(uint32_t pc)
{
    int16_t disp = (int8_t)(currentOpcode & 0x00FF);

    if(disp == 0)
        disp = vdsc->GetWord(pc+2, NoFlags);

    disassembledInstructions.push_back(toHex(pc) + "\tBSR " + toHex(pc + 2 + disp));
}

void SCC68070::DisassembleBtst(uint32_t pc)
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(currentOpcode & 0x0100)
    {
        const uint8_t reg = (currentOpcode & 0x0E00) >> 9;
        data = "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, 1);
    }
    else
    {
        const uint8_t bit = (vdsc->GetWord(pc+2, NoFlags) & 0x00FF) % 8;
        data = "#" + std::to_string(bit) + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, 4);
    }

    disassembledInstructions.push_back(toHex(pc) + "\tBTST " + data);
}

void SCC68070::DisassembleChk(uint32_t pc)
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tCHK " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg));
}

void SCC68070::DisassembleClr(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tCLR" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size));
}

void SCC68070::DisassembleCmp(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tCMP" + (opmode == 1 ? ".B " : opmode == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, opmode) + ", D" + std::to_string(reg));
}

void SCC68070::DisassembleCmpa(uint32_t pc)
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
          uint8_t opmode = (currentOpcode & 0x0100) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(opmode)
        opmode = 4;
    else
        opmode = 2;

    disassembledInstructions.push_back(toHex(pc) + "\tCMPA" + (opmode == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, opmode) + ", A" + std::to_string(reg));
}

void SCC68070::DisassembleCmpi(uint32_t pc)
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string((int8_t)(vdsc->GetByte(pc+3, NoFlags)));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string((int16_t)(vdsc->GetWord(pc+2, NoFlags)));
    }
    else
    {
        size = 4;
        data = std::to_string((int32_t)(vdsc->GetLong(pc+2, NoFlags)));
    }

    disassembledInstructions.push_back(toHex(pc) + "\tCMPI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc + (size == 4 ? size : 2), eamode, eareg, size));
}

void SCC68070::DisassembleCmpm(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tCMPM" + (size == 1 ? ".B (A" : size == 2 ? ".W (A" : ".L (A") + std::to_string(Ay) + ")+, (A" + std::to_string(Ax) + ")+");
}

void SCC68070::DisassembleDbCC(uint32_t pc)
{
    const uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    const uint8_t       reg = (currentOpcode & 0x0007);
    const int16_t      disp = vdsc->GetWord(pc+2, NoFlags);
    disassembledInstructions.push_back(toHex(pc) + "\tDB" + DisassembleConditionalCode(condition) + " D" + std::to_string(reg) + ", " + toHex(pc + 2 + disp));
}

void SCC68070::DisassembleDivs(uint32_t pc)
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tDIVS.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg));
}

void SCC68070::DisassembleDivu(uint32_t pc)
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 6;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tDIVU.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg));
}

void SCC68070::DisassembleEor(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tEOR" + (opmode == 1 ? ".B D" : opmode == 2 ? ".W D" : ".L D") + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, opmode));
}

void SCC68070::DisassembleEori(uint32_t pc)
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string(vdsc->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string(vdsc->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string(vdsc->GetLong(pc+2, NoFlags));
    }

    disassembledInstructions.push_back(toHex(pc) + "\tEORI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc+(size == 4 ? size : 2), eamode, eareg, size));
}

void SCC68070::DisassembleEoriccr(uint32_t pc)
{
    const uint8_t data = vdsc->GetWord(pc+2, NoFlags) & 0x1F;
    disassembledInstructions.push_back(toHex(pc) + "\tEORI #0x" + toHex(data) + ", CCR");
}

void SCC68070::DisassembleEorisr(uint32_t pc)
{
    const uint8_t data = vdsc->GetWord(pc+2, NoFlags);
    disassembledInstructions.push_back(toHex(pc) + "\tEORI #0x" + toHex(data) + ", SR");
}

void SCC68070::DisassembleExg(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tEXG " + left + right);
}

void SCC68070::DisassembleExt(uint32_t pc)
{
    const uint8_t opmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t    reg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tEXT." + (opmode == 2 ? "W D" : "L D") + std::to_string(reg));
}

void SCC68070::DisassembleIllegal(uint32_t pc)
{
    disassembledInstructions.push_back(toHex(pc) + "\tILLEGAL");
}

void SCC68070::DisassembleJmp(uint32_t pc)
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tJMP " + DisassembleAddressingMode(pc+2, eamode, eareg, 0));
}

void SCC68070::DisassembleJsr(uint32_t pc)
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tJSR " + DisassembleAddressingMode(pc+2, eamode, eareg, 0));
}

void SCC68070::DisassembleLea(uint32_t pc)
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tLEA " + DisassembleAddressingMode(pc+2, eamode, eareg, 4) + ", A" + std::to_string(reg));
}

void SCC68070::DisassembleLink(uint32_t pc)
{
    const uint8_t reg = (currentOpcode & 0x0007);
    const int16_t disp = vdsc->GetWord(pc+2, NoFlags);
    disassembledInstructions.push_back(toHex(pc) + "\tLINK A" + std::to_string(reg) + ", #" + std::to_string(disp));
}

void SCC68070::DisassembleLsM(uint32_t pc)
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(dr)
        disassembledInstructions.push_back(toHex(pc) + "\tLSL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2));
    else
        disassembledInstructions.push_back(toHex(pc) + "\tLSR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2));
}

void SCC68070::DisassembleLsR(uint32_t pc)
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
        disassembledInstructions.push_back(toHex(pc) + "\tLSL" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg));
    else
        disassembledInstructions.push_back(toHex(pc) + "\tLSR" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg));
}

void SCC68070::DisassembleMove(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tMOVE." + ((size == 1) ? "B " : (size == 3) ? "W " : "L ") + srcEA + ", " + dstEA);
}

void SCC68070::DisassembleMovea(uint32_t pc)
{
    const uint8_t   size = (currentOpcode & 0x3000) >> 12;
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tMOVEA" + (size == 3 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size == 3 ? 2 : 4, true) + ", A" + std::to_string(reg));
}

void SCC68070::DisassembleMoveccr(uint32_t pc)
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tMOVE " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", CCR");
}

void SCC68070::DisassembleMoveFsr(uint32_t pc)
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tMOVE SR, " + DisassembleAddressingMode(pc+2, eamode, eareg, 2));
}

void SCC68070::DisassembleMovesr(uint32_t pc)
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tMOVE " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", SR");
}

void SCC68070::DisassembleMoveusp(uint32_t pc)
{
    const uint8_t  dr = (currentOpcode & 0x0008) >> 3;
    const uint8_t reg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tMOVE " + (dr ? "USP, A" + std::to_string(reg) : "A" + std::to_string(reg) + ", USP"));
}

void SCC68070::DisassembleMovem(uint32_t pc)
{
    const uint8_t     dr = (currentOpcode & 0x0400) >> 10;
    const uint8_t   size = (currentOpcode & 0x0040) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    const uint16_t mask = vdsc->GetWord(pc+2, NoFlags);

    std::string list;
    list = toBinString(mask >> 8, 8) + " " + toBinString(mask, 8);

    disassembledInstructions.push_back(toHex(pc) + "\tMOVEM" + (size ? ".L " : ".W ") + (dr ? DisassembleAddressingMode(pc+4, eamode, eareg, size ? 4 : 2) + ", " + list : list + ", " + DisassembleAddressingMode(pc+4, eamode, eareg, size ? 4 : 2)));
}

void SCC68070::DisassembleMovep(uint32_t pc)
{
    const uint8_t datareg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t  opmode = (currentOpcode & 0x01C0) >> 6;
    const uint8_t addrreg = (currentOpcode & 0x0007);
    const int16_t    disp = vdsc->GetWord(pc+2, NoFlags);

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

    disassembledInstructions.push_back(toHex(pc) + "\tMOVEP" + operands);
}

void SCC68070::DisassembleMoveq(uint32_t pc)
{
    const uint8_t reg = (currentOpcode & 0x0E00) >> 9;
    const int8_t data = (currentOpcode & 0x00FF);
    disassembledInstructions.push_back(toHex(pc) + "\tMOVEQ #" + std::to_string(data) + ", D" + std::to_string(reg));
}

void SCC68070::DisassembleMuls(uint32_t pc)
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tMULS.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg));
}

void SCC68070::DisassembleMulu(uint32_t pc)
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tMULU.W " + DisassembleAddressingMode(pc+2, eamode, eareg, 2) + ", D" + std::to_string(reg));
}

void SCC68070::DisassembleNbcd(uint32_t pc)
{
    const uint8_t mode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  reg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tNBCD " + DisassembleAddressingMode(pc+2, mode, reg, 1));
}

void SCC68070::DisassembleNeg(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tNEG" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size));
}

void SCC68070::DisassembleNegx(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tNEGX" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size));
}

void SCC68070::DisassembleNop(uint32_t pc)
{
    disassembledInstructions.push_back(toHex(pc) + "\tNOP");
}

void SCC68070::DisassembleNot(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tNOT" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, size));
}

void SCC68070::DisassembleOr(uint32_t pc)
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
    const uint8_t opmode = (currentOpcode & 0x001C) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    uint8_t size;
    if(opmode == 0 || opmode == 4)
        size = 1;
    else if(opmode == 1 || opmode == 5)
        size = 2;
    else
        size = 4;

    disassembledInstructions.push_back(toHex(pc) + "\tOR" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (opmode < 3 ? DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg) : \
                                                                                                                      "D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size)));
}

void SCC68070::DisassembleOri(uint32_t pc)
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string(vdsc->GetByte(pc+3, NoFlags));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string(vdsc->GetWord(pc+2, NoFlags));
    }
    else
    {
        size = 4;
        data = std::to_string(vdsc->GetLong(pc+2, NoFlags));
    }

    disassembledInstructions.push_back(toHex(pc) + "\tORI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc+(size == 4 ? size : 2), eamode, eareg, size));
}

void SCC68070::DisassembleOriccr(uint32_t pc)
{
    const uint8_t data = vdsc->GetWord(pc+2, NoFlags) & 0x1F;
    disassembledInstructions.push_back(toHex(pc) + "\tORI #0x" + toHex(data) + ", CCR");
}

void SCC68070::DisassembleOrisr(uint32_t pc)
{
    const uint16_t data = vdsc->GetWord(pc+2, NoFlags);
    disassembledInstructions.push_back(toHex(pc) + "\tORI #0x" + toHex(data) + ", SR");
}

void SCC68070::DisassemblePea(uint32_t pc)
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tPEA " + DisassembleAddressingMode(pc+2, eamode, eareg, 4));
}

void SCC68070::DisassembleReset(uint32_t pc)
{
    disassembledInstructions.push_back(toHex(pc) + "\tRESET");
}

void SCC68070::DisassembleRoM(uint32_t pc)
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(dr)
        disassembledInstructions.push_back(toHex(pc) + "\tROL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2));
    else
        disassembledInstructions.push_back(toHex(pc) + "\tROR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2));
}

void SCC68070::DisassembleRoR(uint32_t pc)
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
        disassembledInstructions.push_back(toHex(pc) + "\tROL" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg));
    else
        disassembledInstructions.push_back(toHex(pc) + "\tROR" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg));
}

void SCC68070::DisassembleRoxM(uint32_t pc)
{
    const uint8_t     dr = (currentOpcode & 0x0100) >> 8;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(dr)
        disassembledInstructions.push_back(toHex(pc) + "\tROXL " + DisassembleAddressingMode(pc+2, eamode, eareg, 2));
    else
        disassembledInstructions.push_back(toHex(pc) + "\tROXR " + DisassembleAddressingMode(pc+2, eamode, eareg, 2));
}

void SCC68070::DisassembleRoxR(uint32_t pc)
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
        disassembledInstructions.push_back(toHex(pc) + "\tROXL" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg));
    else
        disassembledInstructions.push_back(toHex(pc) + "\tROXR" + (size == 0 ? ".B " : size == 1 ? ".W " : ".L ") + leftOperand + ", D" + std::to_string(reg));
}

void SCC68070::DisassembleRte(uint32_t pc)
{
    disassembledInstructions.push_back(toHex(pc) + "\tRTE");
}

void SCC68070::DisassembleRtr(uint32_t pc)
{
    disassembledInstructions.push_back(toHex(pc) + "\tRTR");
}

void SCC68070::DisassembleRts(uint32_t pc)
{
    disassembledInstructions.push_back(toHex(pc) + "\tRTS");
}

void SCC68070::DisassembleSbcd(uint32_t pc)
{
    const uint8_t Ry = (currentOpcode & 0x0E00) >> 9;
    const uint8_t rm = (currentOpcode & 0x0008);
    const uint8_t Rx = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tSBCD " + (rm ? "-(A" + std::to_string(Rx) + "), -(A" + std::to_string(Ry) + ")" : \
                                                                 "D" + std::to_string(Rx) + ", D" + std::to_string(Ry)));
}

void SCC68070::DisassembleSCC(uint32_t pc)
{
    const uint8_t condition = (currentOpcode & 0x0F00) >> 8;
    const uint8_t    eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t     eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tS" + DisassembleConditionalCode(condition) + " " + DisassembleAddressingMode(pc+2, eamode, eareg, 1));
}

void SCC68070::DisassembleStop(uint32_t pc)
{
    const uint16_t data = vdsc->GetWord(pc+2, NoFlags);
    disassembledInstructions.push_back(toHex(pc) + "\tSTOP #0x" + toHex(data));
}

void SCC68070::DisassembleSub(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tSUB" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (opmode < 3 ? (DisassembleAddressingMode(pc+2, eamode, eareg, size) + ", D" + std::to_string(reg)) : \
                                                                                                                       ("D" + std::to_string(reg) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size))));
}

void SCC68070::DisassembleSuba(uint32_t pc)
{
    const uint8_t    reg = (currentOpcode & 0x0E00) >> 9;
          uint8_t opmode = (currentOpcode & 0x0100) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    if(opmode)
        opmode = 4;
    else
        opmode = 2;

    disassembledInstructions.push_back(toHex(pc) + "\tSUBA" + (opmode == 2 ? ".W " : ".L ") + DisassembleAddressingMode(pc+2, eamode, eareg, opmode) + ", A" + std::to_string(reg));
}

void SCC68070::DisassembleSubi(uint32_t pc)
{
          uint8_t   size = (currentOpcode & 0x00C0) >> 6;
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);

    std::string data;
    if(size == 0)
    {
        size = 1;
        data = std::to_string((int8_t)(vdsc->GetByte(pc+3, NoFlags)));
    }
    else if(size == 1)
    {
        size = 2;
        data = std::to_string((int16_t)(vdsc->GetWord(pc+2, NoFlags)));
    }
    else
    {
        size = 4;
        data = std::to_string((int32_t)(vdsc->GetLong(pc+2, NoFlags)));
    }

    disassembledInstructions.push_back(toHex(pc) + "\tSUBI" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + data + ", " + DisassembleAddressingMode(pc+ (size == 4 ? size : 2), eamode, eareg, size));
}

void SCC68070::DisassembleSubq(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tSUBQ" + (size == 1 ? ".B #" : size == 2 ? ".W #" : ".L #") + std::to_string(data) + ", " + DisassembleAddressingMode(pc+2, eamode, eareg, size));
}

void SCC68070::DisassembleSubx(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tSUBX" + (size == 1 ? ".B " : size == 2 ? ".W " : ".L ") + (rm ? "-(A" + std::to_string(rx) + "), -(A" + std::to_string(ry) + ")" : \
                                                                                                                  "D" + std::to_string(rx) + ", D" + std::to_string(ry)));
}

void SCC68070::DisassembleSwap(uint32_t pc)
{
    disassembledInstructions.push_back(toHex(pc) + "\tSWAP D" + std::to_string(currentOpcode & 0x0007));
}

void SCC68070::DisassembleTas(uint32_t pc)
{
    const uint8_t eamode = (currentOpcode & 0x0038) >> 3;
    const uint8_t  eareg = (currentOpcode & 0x0007);
    disassembledInstructions.push_back(toHex(pc) + "\tTAS " + DisassembleAddressingMode(pc+2, eamode, eareg, 1));
}

void SCC68070::DisassembleTrap(uint32_t pc)
{
    disassembledInstructions.push_back(toHex(pc) + "\tTRAP #" + std::to_string(currentOpcode & 0x000F));
}

void SCC68070::DisassembleTrapv(uint32_t pc)
{
    disassembledInstructions.push_back(toHex(pc) + "\tTRAPV");
}

void SCC68070::DisassembleTst(uint32_t pc)
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

    disassembledInstructions.push_back(toHex(pc) + "\tTST " + DisassembleAddressingMode(pc+2, eamode, eareg, size));
}

void SCC68070::DisassembleUnlk(uint32_t pc)
{
    disassembledInstructions.push_back(toHex(pc) + "\tUNLK A" + std::to_string(currentOpcode & 0x0007));
}
