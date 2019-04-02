#include "SCC68070.hpp"

void SCC68070::Interpreter()
{
    if(stop)
        return;
    if(executionTime > 1000)
    {
        instructionsBuffer = "";
        executionTime = 0;
    }

    currentOpcode = GetNextWord();
    uint16_t opcode = GetInstructionIf(currentOpcode);
    instructionsBuffer += " " + std::to_string(opcode) + " ";
    executionTime += (this->*instructions[opcode])();
    instructionsBufferChanged = true;
    count++;
}

uint16_t SCC68070::GetInstructionIf(const uint16_t& opcode)
{
    if(opcode == 0x4E70) return RESET;
    if(opcode == 0x4E71) return NOP;
    if(opcode == 0x4E72) return STOP;
    if(opcode == 0x4E73) return RTE;
    if(opcode == 0x4E75) return RTS;
    if(opcode == 0x4E76) return TRAPV;
    if(opcode == 0x4E77) return RTR;
    if((opcode & 0xF1F0) == 0xC100) return ABCD;
    if((opcode & 0xF0C0) == 0xD0C0) return ADDA;
    if((opcode & 0xF000) == 0xD000) return ADD;
    if((opcode & 0xFF00) == 0x0600) return ADDI;
    if((opcode & 0xF100) == 0x5000) return ADDQ;
    if((opcode & 0xF130) == 0xD100) return ADDX;
    if((opcode & 0xF000) == 0xC000) return AND;
    if((opcode & 0xFF00) == 0x0200) return ANDI;
    if((opcode & 0xFEC0) == 0xE0C0) return ASm;
    if((opcode & 0xF018) == 0xE000) return ASr;
    if((opcode & 0xFF00) == 0x6000) return BRA;
    if((opcode & 0xFF00) == 0x6100) return BSR;
    if((opcode & 0xF000) == 0x6000) return Bcc;
    if((opcode & 0xF1C0) == 0x0140) return BCHG;
    if((opcode & 0xFFC0) == 0x0840) return BCHG;
    if((opcode & 0xF1C0) == 0x0180) return BCLR;
    if((opcode & 0xFFC0) == 0x0880) return BCLR;
    if((opcode & 0xF1C0) == 0x01C0) return BSET;
    if((opcode & 0xFFC0) == 0x08C0) return BSET;
    if((opcode & 0xF1C0) == 0x0100) return BTST;
    if((opcode & 0xFFC0) == 0x0800) return BTST;
    if((opcode & 0xF140) == 0x4100) return CHK;
    if((opcode & 0xFF00) == 0x4200) return CLR;
    if((opcode & 0xF100) == 0xB000) return CMP;
    if((opcode & 0xF0C0) == 0xB0C0) return CMPA;
    if((opcode & 0xFF00) == 0x0C00) return CMPI;
    if((opcode & 0xF138) == 0xB108) return CMPM;
    if((opcode & 0xF0F8) == 0x50C8) return DBcc;
    if((opcode & 0xF1C0) == 0x81C0) return DIVS;
    if((opcode & 0xF1C0) == 0x80C0) return DIVU;
    if((opcode & 0xF100) == 0xB100) return EOR;
    if((opcode & 0xFF00) == 0x0A00) return EORI;
    if((opcode & 0xF130) == 0xC100) return EXG;
    if((opcode & 0xFFC0) == 0x4EC0) return JMP;
    if((opcode & 0xFFC0) == 0x4E80) return JSR;
    if((opcode & 0xF1C0) == 0x41C0) return LEA;
    if((opcode & 0xFFF8) == 0x4E50) return LINK;
    if((opcode & 0xF018) == 0xE008) return LSr;
    if((opcode & 0xFEC0) == 0xE2C0) return LSm;
    if((opcode & 0xC000) == 0x0000 && (opcode & 0xF000) != 0x0000) return MOVE;
    if((opcode & 0xE1C0) == 0x2040) return MOVEA;
    if((opcode & 0xFFC0) == 0x44C0) return MOVECCR;
    if((opcode & 0xFFC0) == 0x40C0) return MOVEfSR;
    if((opcode & 0xFFC0) == 0x46C0) return MOVESR;
    if((opcode & 0xFFF0) == 0x4E60) return MOVEUSP;
    if((opcode & 0xFB80) == 0x4880 && (opcode & 0xFBB8) != 0x4880) return MOVEM;
    if((opcode & 0xFEB8) == 0x4880) return EXT;
    if((opcode & 0xF138) == 0x0108) return MOVEP;
    if((opcode & 0xF100) == 0x7000) return MOVEQ;
    if((opcode & 0xF1C0) == 0xC1C0) return MULS;
    if((opcode & 0xF1C0) == 0xC0C0) return MULU;
    if((opcode & 0xFFC0) == 0x4800) return NBCD;
    if((opcode & 0xFF00) == 0x4400) return NEG;
    if((opcode & 0xFF00) == 0x4000) return NEGX;
    if((opcode & 0xFF00) == 0x4600) return NOT;
    if((opcode & 0xF000) == 0x4000) return OR;
    if((opcode & 0xFF00) == 0x0000) return ORI;
    if((opcode & 0xF018) == 0xE018) return ROr;
    if((opcode & 0xFEC0) == 0xE6C0 && (opcode & 0xFEF8) != 0xE6C0) return ROm;
    if((opcode & 0xF018) == 0xE010) return ROXr;
    if((opcode & 0xFEC0) == 0xE4C0 && (opcode & 0xFEF8) != 0xE4C0) return ROXm;
    if((opcode & 0xF1F0) == 0x8100) return SBCD;
    if((opcode & 0xF0C0) == 0x50C0) return Scc;
    if((opcode & 0xF000) == 0x9000) return SUB;
    if((opcode & 0xF0C0) == 0x90C0) return SUBA;
    if((opcode & 0xFF00) == 0x0400) return SUBI;
    if((opcode & 0xF100) == 0x5100) return SUBQ;
    if((opcode & 0xF130) == 0x9100) return SUBX;
    if((opcode & 0xFFF8) == 0x4840) return SWAP;
    if((opcode & 0xFFC0) == 0x4840) return PEA;
    if((opcode & 0xFFC0) == 0x4AC0) return TAS;
    if((opcode & 0xFFF0) == 0x4E40) return TRAP;
    if((opcode & 0xFF00) == 0x4A00) return TST;
    if((opcode & 0xFFF8) == 0x4E58) return UNLK;
    return UNKNOWN;
}
