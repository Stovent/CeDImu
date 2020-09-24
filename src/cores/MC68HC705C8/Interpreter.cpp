#include "MC68HC705C8.hpp"
#include "../../utils.hpp"

#include <wx/msgdlg.h>

void MC68HC705C8::Execute(const int cycles)
{
    if(waitStop)
        return;

    pendingCycles += cycles;
    while(pendingCycles > 0)
    {
        currentPC = PC;
        currentOpcode = GetNextByte();

        pendingCycles -= IndirectThreadedCode();
    }
}

uint8_t MC68HC705C8::IndirectThreadedCode()
{
    const void* ITC[256] = {
        &&BRSET0_DIR, &&BRCLR0_DIR, &&BRSET1_DIR, &&BRCLR1_DIR, &&BRSET2_DIR, &&BRCLR2_DIR, &&BRSET3_DIR, &&BRCLR3_DIR, &&BRSET4_DIR,   &&BRCLR4_DIR, &&BRSET5_DIR, &&BRCLR5_DIR, &&BRSET6_DIR, &&BRCLR6_DIR, &&BRSET7_DIR, &&BRCLR7_DIR, // 0x0X
        &&BSET0_DIR,  &&BCLR0_DIR,  &&BSET1_DIR,  &&BCLR1_DIR,  &&BSET2_DIR,  &&BCLR2_DIR,  &&BSET3_DIR,  &&BCLR3_DIR,  &&BSET4_DIR,    &&BCLR4_DIR,  &&BSET5_DIR,  &&BCLR5_DIR,  &&BSET6_DIR,  &&BCLR6_DIR,  &&BSET7_DIR,  &&BCLR7_DIR,  // 0x1X
        &&BRA_REL,    &&BRN_REL,    &&BHI_REL,    &&BLS_REL,    &&BCC_REL,    &&BCSBLO_REL, &&BNE_REL,    &&BEQ_REL,    &&BHCC_REL,     &&BHCS_REL,   &&BPL_REL,    &&BMI_REL,    &&BMC_REL,    &&BMS_REL,    &&BIL_REL,    &&BIH_REL,    // 0x2X
        &&NEG_DIR,    &&unknown,    &&unknown,    &&COM_DIR,    &&LSR_DIR,    &&unknown,    &&ROR_DIR,    &&ASR_DIR,    &&ASLLSL_DIR,   &&ROL_DIR,    &&DEC_DIR,    &&unknown,    &&INC_DIR,    &&TST_DIR,    &&unknown,    &&CLR_DIR,    // 0x3X
        &&NEGA_INH,   &&unknown,    &&MUL_INH,    &&COMA_INH,   &&LSRA_INH,   &&unknown,    &&RORA_INH,   &&ASRA_INH,   &&ASLALSLA_INH, &&ROLA_INH,   &&DECA_INH,   &&unknown,    &&INCA_INH,   &&TSTA_INH,   &&unknown,    &&CLRA_INH,   // 0x4X
        &&NEGX_INH,   &&unknown,    &&unknown,    &&COMX_INH,   &&LSRX_INH,   &&unknown,    &&RORX_INH,   &&ASRX_INH,   &&ASLXLSLX_INH, &&ROLX_INH,   &&DECX_INH,   &&unknown,    &&INCX_INH,   &&TSTX_INH,   &&unknown,    &&CLRX_INH,   // 0x5X
        &&NEG_IX1,    &&unknown,    &&unknown,    &&COM_IX1,    &&LSR_IX1,    &&unknown,    &&ROR_IX1,    &&ASR_IX1,    &&ASLLSL_IX1,   &&ROL_IX1,    &&DEC_IX1,    &&unknown,    &&INC_IX1,    &&TST_IX1,    &&unknown,    &&CLR_IX1,    // 0x6X
        &&NEG_IX,     &&unknown,    &&unknown,    &&COM_IX,     &&LSR_IX,     &&unknown,    &&ROR_IX,     &&ASR_IX,     &&ASLLSL_IX,    &&ROL_IX,     &&DEC_IX,     &&unknown,    &&INC_IX,     &&TST_IX,     &&unknown,    &&CLR_IX,     // 0x7X
        &&RTI_INH,    &&RTS_INH,    &&unknown,    &&SWI_INH,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,      &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&STOP_INH,   &&WAIT_INH,   // 0x8X
        &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&TAX_INH,    &&CLC_INH,      &&SEC_INH,    &&CLI_INH,    &&SEI_INH,    &&RSP_INH,    &&NOP_INH,    &&unknown,    &&TXA_INH,    // 0x9X
        &&SUB_IMM,    &&CMP_IMM,    &&SBC_IMM,    &&CPX_IMM,    &&AND_IMM,    &&BIT_IMM,    &&LDA_IMM,    &&unknown,    &&EOR_IMM,      &&ADC_IMM,    &&ORA_IMM,    &&ADD_IMM,    &&unknown,    &&BSR_REL,    &&LDX_IMM,    &&unknown,    // 0xAX
        &&SUB_DIR,    &&CMP_DIR,    &&SBC_DIR,    &&CPX_DIR,    &&AND_DIR,    &&BIT_DIR,    &&LDA_DIR,    &&STA_DIR,    &&EOR_DIR,      &&ADC_DIR,    &&ORA_DIR,    &&ADD_DIR,    &&JMP_DIR,    &&JSR_DIR,    &&LDX_DIR,    &&SDX_DIR,    // 0xBX
        &&SUB_EXT,    &&CMP_EXT,    &&SBC_EXT,    &&CPX_EXT,    &&AND_EXT,    &&BIT_EXT,    &&LDA_EXT,    &&STA_EXT,    &&EOR_EXT,      &&ADC_EXT,    &&ORA_EXT,    &&ADD_EXT,    &&JMP_EXT,    &&JSR_EXT,    &&LDX_EXT,    &&SDX_EXT,    // 0xCX
        &&SUB_IX2,    &&CMP_IX2,    &&SBC_IX2,    &&CPX_IX2,    &&AND_IX2,    &&BIT_IX2,    &&LDA_IX2,    &&STA_IX2,    &&EOR_IX2,      &&ADC_IX2,    &&ORA_IX2,    &&ADD_IX2,    &&JMP_IX2,    &&JSR_IX2,    &&LDX_IX2,    &&SDX_IX2,    // 0xDX
        &&SUB_IX1,    &&CMP_IX1,    &&SBC_IX1,    &&CPX_IX1,    &&AND_IX1,    &&BIT_IX1,    &&LDA_IX1,    &&STA_IX1,    &&EOR_IX1,      &&ADC_IX1,    &&ORA_IX1,    &&ADD_IX1,    &&JMP_IX1,    &&JSR_IX1,    &&LDX_IX1,    &&SDX_IX1,    // 0xEX
        &&SUB_IX,     &&CMP_IX,     &&SBC_IX,     &&CPX_IX,     &&AND_IX,     &&BIT_IX,     &&LDA_IX,     &&STA_IX,     &&EOR_IX,       &&ADC_IX,     &&ORA_IX,     &&ADD_IX,     &&JMP_IX,     &&JSR_IX,     &&LDX_IX,     &&STX_IX,     // 0xFX
    };

    goto *ITC[currentOpcode];
        LOG(instructions << std::hex << currentPC << "\t" << std::endl)

    // 0x0X
    BRSET0_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x01)
        {
            PC += offset;
            CCR[C] = true;
        }
        else
            CCR[C] = false;
        LOG(instructions << std::hex << currentPC << "\tBRSET0 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRCLR0_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x01)
            CCR[C] = true;
        else
        {
            PC += offset;
            CCR[C] = false;
        }
        LOG(instructions << std::hex << currentPC << "\tBRCLR0 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRSET1_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x02)
        {
            PC += offset;
            CCR[C] = true;
        }
        else
            CCR[C] = false;
        LOG(instructions << std::hex << currentPC << "\tBRSET1 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRCLR1_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x02)
            CCR[C] = true;
        else
        {
            PC += offset;
            CCR[C] = false;
        }
        LOG(instructions << std::hex << currentPC << "\tBRCLR1 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRSET2_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x04)
        {
            PC += offset;
            CCR[C] = true;
        }
        else
            CCR[C] = false;
        LOG(instructions << std::hex << currentPC << "\tBRSET2 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRCLR2_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x04)
            CCR[C] = true;
        else
        {
            PC += offset;
            CCR[C] = false;
        }
        LOG(instructions << std::hex << currentPC << "\tBRCLR2 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRSET3_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x08)
        {
            PC += offset;
            CCR[C] = true;
        }
        else
            CCR[C] = false;
        LOG(instructions << std::hex << currentPC << "\tBRSET3 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRCLR3_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x08)
            CCR[C] = true;
        else
        {
            PC += offset;
            CCR[C] = false;
        }
        LOG(instructions << std::hex << currentPC << "\tBRCLR3 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRSET4_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x10)
        {
            PC += offset;
            CCR[C] = true;
        }
        else
            CCR[C] = false;
        LOG(instructions << std::hex << currentPC << "\tBRSET4 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRCLR4_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x10)
            CCR[C] = true;
        else
        {
            PC += offset;
            CCR[C] = false;
        }
        LOG(instructions << std::hex << currentPC << "\tBRCLR4 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRSET5_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x20)
        {
            PC += offset;
            CCR[C] = true;
        }
        else
            CCR[C] = false;
        LOG(instructions << std::hex << currentPC << "\tBRSET5 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRCLR5_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x20)
            CCR[C] = true;
        else
        {
            PC += offset;
            CCR[C] = false;
        }
        LOG(instructions << std::hex << currentPC << "\tBRCLR5 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRSET6_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x40)
        {
            PC += offset;
            CCR[C] = true;
        }
        else
            CCR[C] = false;
        LOG(instructions << std::hex << currentPC << "\tBRSET6 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRCLR6_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x40)
            CCR[C] = true;
        else
        {
            PC += offset;
            CCR[C] = false;
        }
        LOG(instructions << std::hex << currentPC << "\tBRCLR6 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRSET7_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x80)
        {
            PC += offset;
            CCR[C] = true;
        }
        else
            CCR[C] = false;
        LOG(instructions << std::hex << currentPC << "\tBRSET7 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    BRCLR7_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t offset = GetNextByte();
        if(GetByte(addr) & 0x80)
            CCR[C] = true;
        else
        {
            PC += offset;
            CCR[C] = false;
        }
        LOG(instructions << std::hex << currentPC << "\tBRCLR7 0x" << addr << ", " << std::dec << offset << std::endl)
        return 5;
    }

    // 0x1X
    BSET0_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x01);
        LOG(instructions << std::hex << currentPC << "\tBSET0 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BCLR0_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xFE);
        LOG(instructions << std::hex << currentPC << "\tBCLR0 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BSET1_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x02);
        LOG(instructions << std::hex << currentPC << "\tBSET1 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BCLR1_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xFD);
        LOG(instructions << std::hex << currentPC << "\tBCLR1 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BSET2_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x04);
        LOG(instructions << std::hex << currentPC << "\tBSET2 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BCLR2_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xFB);
        LOG(instructions << std::hex << currentPC << "\tBCLR2 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BSET3_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x08);
        LOG(instructions << std::hex << currentPC << "\tBSET3 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BCLR3_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xF7);
        LOG(instructions << std::hex << currentPC << "\tBCLR3 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BSET4_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x10);
        LOG(instructions << std::hex << currentPC << "\tBSET4 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BCLR4_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xEF);
        LOG(instructions << std::hex << currentPC << "\tBCLR4 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BSET5_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x20);
        LOG(instructions << std::hex << currentPC << "\tBSET5 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BCLR5_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xDF);
        LOG(instructions << std::hex << currentPC << "\tBCLR5 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BSET6_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x40);
        LOG(instructions << std::hex << currentPC << "\tBSET6 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BCLR6_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xBF);
        LOG(instructions << std::hex << currentPC << "\tBCLR6 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BSET7_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x80);
        LOG(instructions << std::hex << currentPC << "\tBSET7 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    BCLR7_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0x7F);
        LOG(instructions << std::hex << currentPC << "\tBCLR7 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    // 0x2X
    BRA_REL:
    BRN_REL:
    BHI_REL:
    BLS_REL:
    BCC_REL:
    BCSBLO_REL:
    BNE_REL:
    BEQ_REL:
    BHCC_REL:
    BHCS_REL:
    BPL_REL:
    BMI_REL:
    BMC_REL:
    BMS_REL:
    BIL_REL:
    BIH_REL:

    // 0x3X
    NEG_DIR:
    COM_DIR:
    LSR_DIR:
    ROR_DIR:
    ASR_DIR:
    ASLLSL_DIR:
    ROL_DIR:
    DEC_DIR:
    INC_DIR:
    TST_DIR:
    CLR_DIR:

    // 0x4X
    NEGA_INH:
    MUL_INH:
    {
        const uint16_t result = A * X;
        X = (result & 0xFF00) >> 8;
        A = result & 0xFF;
        CCR[H] = 0;
        CCR[C] = 0;
        LOG(instructions << std::hex << currentPC << "\tMUL" << std::endl)
        return 1;
    }

    COMA_INH:
    LSRA_INH:
    RORA_INH:
    ASRA_INH:
    ASLALSLA_INH:
    ROLA_INH:
    DECA_INH:
    INCA_INH:
    TSTA_INH:
    CLRA_INH:

    // 0x5X
    NEGX_INH:
    COMX_INH:
    LSRX_INH:
    RORX_INH:
    ASRX_INH:
    ASLXLSLX_INH:
    ROLX_INH:
    DECX_INH:
    INCX_INH:
    TSTX_INH:
    CLRX_INH:

    // 0x6X
    NEG_IX1:
    COM_IX1:
    LSR_IX1:
    ROR_IX1:
    ASR_IX1:
    ASLLSL_IX1:
    ROL_IX1:
    DEC_IX1:
    INC_IX1:
    TST_IX1:
    CLR_IX1:

    // 0x7X
    NEG_IX:
    COM_IX:
    LSR_IX:
    ROR_IX:
    ASR_IX:
    ASLLSL_IX:
    ROL_IX:
    DEC_IX:
    INC_IX:
    TST_IX:
    CLR_IX:

    // 0x8X
    RTI_INH:
    {
        CCR = PopByte();
        A = PopByte();
        X = PopByte();
        PC = PopByte() << 8;
        PC |= PopByte();
        LOG(instructions << std::hex << currentPC << "\tRTI" << std::endl)
        return 9;
    }

    RTS_INH:
    {
        PC = PopByte() << 8;
        PC |= PopByte();
        LOG(instructions << std::hex << currentPC << "\tRTS" << std::endl)
        return 6;
    }

    SWI_INH:
    {
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PushByte(X);
        PushByte(A);
        PushByte(CCR.to_ulong());
        CCR[I] = true;
        PC = GetByte(0x1FFC) << 8;
        PC |= GetByte(0x1FFD);
        LOG(instructions << std::hex << currentPC << "\tSWI" << std::endl)
        return 1 ? 0 : 0;
    }

    STOP_INH:
    {
        waitStop = true;
        pendingCycles = 2;
        CCR[I] = false;
        LOG(instructions << std::hex << currentPC << "\tSTOP" << std::endl)
        return 2;
    }

    WAIT_INH:
    {
        waitStop = true;
        pendingCycles = 2;
        CCR[I] = false;
        LOG(instructions << std::hex << currentPC << "\tWAIT" << std::endl)
        return 2;
    }

    // 0x9X
    TAX_INH:
        X = A;
        LOG(instructions << std::hex << currentPC << "\tTAX" << std::endl)
        return 2;

    CLC_INH:
        CCR[C] = 0;
        LOG(instructions << std::hex << currentPC << "\tCLC" << std::endl)
        return 2;

    SEC_INH:
        CCR[C] = 1;
        LOG(instructions << std::hex << currentPC << "\tSEC" << std::endl)
        return 2;

    CLI_INH:
        CCR[I] = 0;
        LOG(instructions << std::hex << currentPC << "\tCLI" << std::endl)
        return 2;

    SEI_INH:
        CCR[I] = 1;
        LOG(instructions << std::hex << currentPC << "\tSEI" << std::endl)
        return 2;

    RSP_INH:
        SP = 0xFF;
        LOG(instructions << std::hex << currentPC << "\tRSP" << std::endl)
        return 2;

    NOP_INH:
        LOG(instructions << std::hex << currentPC << "\tNOP" << std::endl)
        return 2;

    TXA_INH:
        A = X;
        LOG(instructions << std::hex << currentPC << "\tTAX" << std::endl)
        return 2;

    // 0xAX
    SUB_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = (result == 0) ? true : false;
        CCR[C] = (result & 0xFF00) ? true : false; // TODO: check how to detect borrow
        A = result & 0x00FF;
        LOG(instructions << std::hex << currentPC << "\tSUB #0x" << (uint16_t)data << std::endl)
        return 2;
    }

    CMP_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = (result == 0) ? true : false;
        CCR[C] = (result & 0xFF00) ? true : false; // TODO: check how to detect borrow
        LOG(instructions << std::hex << currentPC << "\tCMP #0x" << (uint16_t)data << std::endl)
        return 2;
    }

    SBC_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = A - data - CCR[C];
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = (result == 0) ? true : false;
        CCR[C] = (result & 0xFF00) ? true : false; // TODO: check how to detect borrow
        A = result & 0x00FF;
        LOG(instructions << std::hex << currentPC << "\tSBC #0x" << (uint16_t)data << std::endl)
        return 2;
    }

    CPX_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = X - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = (result == 0) ? true : false;
        CCR[C] = (result & 0xFF00) ? true : false; // TODO: check how to detect borrow
        LOG(instructions << std::hex << currentPC << "\tCPX #0x" << (uint16_t)data << std::endl)
        return 2;
    }

    AND_IMM:
    {
        const uint8_t data = GetNextByte();
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = (A == 0) ? true : false;
        LOG(instructions << std::hex << currentPC << "\tAND #0x" << (uint16_t)data << std::endl)
        return 2;
    }

    BIT_IMM:
    {
        uint8_t data = GetNextByte();
        LOG(instructions << std::hex << currentPC << "\tBIT #0x" << (uint16_t)data << std::endl)
        data &= A;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = (data == 0) ? true : false;
        return 2;
    }

    LDA_IMM:
    {
        A = GetNextByte();
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = (A == 0) ? true : false;
        LOG(instructions << std::hex << currentPC << "\tLDA #0x" << (uint16_t)A << std::endl)
        return 2;
    }

    EOR_IMM:
    {
        const uint8_t data = GetNextByte();
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = (A == 0) ? true : false;
        LOG(instructions << std::hex << currentPC << "\tEOR #0x" << (uint16_t)data << std::endl)
        return 2;
    }

    ADC_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = A + data + CCR[C];
        CCR[H] = ((A & 0xF) + (data & 0xF) + CCR[C]) & 0x10 ? true : false;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = (result == 0) ? true : false;
        CCR[C] = (result & 0xFF00) ? true : false;
        LOG(instructions << std::hex << currentPC << "\tADD #0x" << (uint16_t)data << std::endl)
        A = result & 0x00FF;
        return 2;
    }

    ORA_IMM:
    {
        const uint8_t data = GetNextByte();
        A |= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = (A == 0) ? true : false;
        LOG(instructions << std::hex << currentPC << "\tORA #0x" << (uint16_t)data << std::endl)
        return 2;
    }

    ADD_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = A + data;
        CCR[H] = ((A & 0xF) + (data & 0xF)) & 0x10 ? true : false;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = (result == 0) ? true : false;
        CCR[C] = (result & 0xFF00) ? true : false;
        LOG(instructions << std::hex << currentPC << "\tADD #0x" << (uint16_t)data << std::endl)
        A = result & 0x00FF;
        return 2;
    }

    BSR_REL:
    {
        int8_t offset = GetNextByte();
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBSR " << std::dec << (int16_t)offset << std::endl)
        return 6;
    }

    LDX_IMM:
    {
        X = GetNextByte();
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = (X == 0) ? true : false;
        LOG(instructions << std::hex << currentPC << "\tLDX #0x" << (uint16_t)X << std::endl)
        return 2;
    }


    // 0xBX
    SUB_DIR:
    CMP_DIR:
    SBC_DIR:
    CPX_DIR:
    AND_DIR:
    BIT_DIR:
    LDA_DIR:
    STA_DIR:
    EOR_DIR:
    ADC_DIR:
    ORA_DIR:
    ADD_DIR:
    JMP_DIR:
    JSR_DIR:
    LDX_DIR:
    SDX_DIR:

    // 0xCX
    SUB_EXT:
    CMP_EXT:
    SBC_EXT:
    CPX_EXT:
    AND_EXT:
    BIT_EXT:
    LDA_EXT:
    STA_EXT:
    EOR_EXT:
    ADC_EXT:
    ORA_EXT:
    ADD_EXT:
    JMP_EXT:
    JSR_EXT:
    LDX_EXT:
    SDX_EXT:

    // 0xDX
    SUB_IX2:
    CMP_IX2:
    SBC_IX2:
    CPX_IX2:
    AND_IX2:
    BIT_IX2:
    LDA_IX2:
    STA_IX2:
    EOR_IX2:
    ADC_IX2:
    ORA_IX2:
    ADD_IX2:
    JMP_IX2:
    JSR_IX2:
    LDX_IX2:
    SDX_IX2:

    // 0xEX
    SUB_IX1:
    CMP_IX1:
    SBC_IX1:
    CPX_IX1:
    AND_IX1:
    BIT_IX1:
    LDA_IX1:
    STA_IX1:
    EOR_IX1:
    ADC_IX1:
    ORA_IX1:
    ADD_IX1:
    JMP_IX1:
    JSR_IX1:
    LDX_IX1:
    SDX_IX1:

    // 0xFX
    SUB_IX:
    CMP_IX:
    SBC_IX:
    CPX_IX:
    AND_IX:
    BIT_IX:
    LDA_IX:
    STA_IX:
    EOR_IX:
    ADC_IX:
    ORA_IX:
    ADD_IX:
    JMP_IX:
    JSR_IX:
    LDX_IX:
    STX_IX:

    unknown:
        LOG(instructions << std::hex << currentPC << "\tUnknwon instruction: 0x" << (uint16_t)currentOpcode << std::endl)
    return 0;
}
