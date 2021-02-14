#include "MC68HC705C8.hpp"
#include "../../common/utils.hpp"

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
    constexpr void* ITC[256] = {
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
        &&SUB_DIR,    &&CMP_DIR,    &&SBC_DIR,    &&CPX_DIR,    &&AND_DIR,    &&BIT_DIR,    &&LDA_DIR,    &&STA_DIR,    &&EOR_DIR,      &&ADC_DIR,    &&ORA_DIR,    &&ADD_DIR,    &&JMP_DIR,    &&JSR_DIR,    &&LDX_DIR,    &&STX_DIR,    // 0xBX
        &&SUB_EXT,    &&CMP_EXT,    &&SBC_EXT,    &&CPX_EXT,    &&AND_EXT,    &&BIT_EXT,    &&LDA_EXT,    &&STA_EXT,    &&EOR_EXT,      &&ADC_EXT,    &&ORA_EXT,    &&ADD_EXT,    &&JMP_EXT,    &&JSR_EXT,    &&LDX_EXT,    &&STX_EXT,    // 0xCX
        &&SUB_IX2,    &&CMP_IX2,    &&SBC_IX2,    &&CPX_IX2,    &&AND_IX2,    &&BIT_IX2,    &&LDA_IX2,    &&STA_IX2,    &&EOR_IX2,      &&ADC_IX2,    &&ORA_IX2,    &&ADD_IX2,    &&JMP_IX2,    &&JSR_IX2,    &&LDX_IX2,    &&STX_IX2,    // 0xDX
        &&SUB_IX1,    &&CMP_IX1,    &&SBC_IX1,    &&CPX_IX1,    &&AND_IX1,    &&BIT_IX1,    &&LDA_IX1,    &&STA_IX1,    &&EOR_IX1,      &&ADC_IX1,    &&ORA_IX1,    &&ADD_IX1,    &&JMP_IX1,    &&JSR_IX1,    &&LDX_IX1,    &&STX_IX1,    // 0xEX
        &&SUB_IX,     &&CMP_IX,     &&SBC_IX,     &&CPX_IX,     &&AND_IX,     &&BIT_IX,     &&LDA_IX,     &&STA_IX,     &&EOR_IX,       &&ADC_IX,     &&ORA_IX,     &&ADD_IX,     &&JMP_IX,     &&JSR_IX,     &&LDX_IX,     &&STX_IX,     // 0xFX
    };

    goto *ITC[currentOpcode];

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
        LOG(fprintf(instructions, "%X\tBRSET0 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRCLR0 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRSET1 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRCLR1 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRSET2 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRCLR2 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRSET3 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRCLR3 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRSET4 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRCLR4 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRSET5 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRCLR5 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRSET6 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRCLR6 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRSET7 0x%X, %d\n", currentPC, addr, offset);)
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
        LOG(fprintf(instructions, "%X\tBRCLR7 0x%X, %d\n", currentPC, addr, offset);)
        return 5;
    }

    // 0x1X
    BSET0_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x01);
        LOG(fprintf(instructions, "%X\tBSET0 0x%X\n", currentPC, addr);)
        return 5;
    }

    BCLR0_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xFE);
        LOG(fprintf(instructions, "%X\tBCLR0 0x%X\n", currentPC, addr);)
        return 5;
    }

    BSET1_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x02);
        LOG(fprintf(instructions, "%X\tBSET1 0x%X\n", currentPC, addr);)
        return 5;
    }

    BCLR1_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xFD);
        LOG(fprintf(instructions, "%X\tBCLR1 0x%X\n", currentPC, addr);)
        return 5;
    }

    BSET2_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x04);
        LOG(fprintf(instructions, "%X\tBSET2 0x%X\n", currentPC, addr);)
        return 5;
    }

    BCLR2_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xFB);
        LOG(fprintf(instructions, "%X\tBCLR2 0x%X\n", currentPC, addr);)
        return 5;
    }

    BSET3_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x08);
        LOG(fprintf(instructions, "%X\tBSET3 0x%X\n", currentPC, addr);)
        return 5;
    }

    BCLR3_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xF7);
        LOG(fprintf(instructions, "%X\tBCLR3 0x%X\n", currentPC, addr);)
        return 5;
    }

    BSET4_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x10);
        LOG(fprintf(instructions, "%X\tBSET4 0x%X\n", currentPC, addr);)
        return 5;
    }

    BCLR4_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xEF);
        LOG(fprintf(instructions, "%X\tBCLR4 0x%X\n", currentPC, addr);)
        return 5;
    }

    BSET5_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x20);
        LOG(fprintf(instructions, "%X\tBSET5 0x%X\n", currentPC, addr);)
        return 5;
    }

    BCLR5_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xDF);
        LOG(fprintf(instructions, "%X\tBCLR5 0x%X\n", currentPC, addr);)
        return 5;
    }

    BSET6_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x40);
        LOG(fprintf(instructions, "%X\tBSET6 0x%X\n", currentPC, addr);)
        return 5;
    }

    BCLR6_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0xBF);
        LOG(fprintf(instructions, "%X\tBCLR6 0x%X\n", currentPC, addr);)
        return 5;
    }

    BSET7_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) | 0x80);
        LOG(fprintf(instructions, "%X\tBSET7 0x%X\n", currentPC, addr);)
        return 5;
    }

    BCLR7_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, GetByte(addr) & 0x7F);
        LOG(fprintf(instructions, "%X\tBCLR7 0x%X\n", currentPC, addr);)
        return 5;
    }

    // 0x2X
    BRA_REL:
    {
        const int8_t offset = GetNextByte();
        PC += offset;
        LOG(fprintf(instructions, "%X\tBRA %d\n", currentPC, offset);)
        return 3;
    }

    BRN_REL:
    {
        LOG(fprintf(instructions, "%X\tBRN %d\n", currentPC, signExtend<int8_t, int16_t>(GetNextByte()));)
        return 3;
    }

    BHI_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[C] && !CCR[Z])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBHI %d\n", currentPC, offset);)
        return 3;
    }

    BLS_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[C] || CCR[Z])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBLS %d\n", currentPC, offset);)
        return 3;
    }

    BCC_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[C])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBCC %d\n", currentPC, offset);)
        return 3;
    }

    BCSBLO_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[C])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBLO/BCS %d\n", currentPC, offset);)
        return 3;
    }

    BNE_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[Z])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBNE %d\n", currentPC, offset);)
        return 3;
    }

    BEQ_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[Z])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBEQ %d\n", currentPC, offset);)
        return 3;
    }

    BHCC_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[H])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBHCC %d\n", currentPC, offset);)
        return 3;
    }

    BHCS_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[H])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBHCS %d\n", currentPC, offset);)
        return 3;
    }

    BPL_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[N])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBPL %d\n", currentPC, offset);)
        return 3;
    }

    BMI_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[N])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBMI %d\n", currentPC, offset);)
        return 3;
    }

    BMC_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[I])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBMC %d\n", currentPC, offset);)
        return 3;
    }

    BMS_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[I])
            PC += offset;
        LOG(fprintf(instructions, "%X\tBMS %d\n", currentPC, offset);)
        return 3;
    }

    BIL_REL:
    {
        const int8_t offset = GetNextByte();
        if(!irqPin)
            PC += offset;
        LOG(fprintf(instructions, "%X\tBIL %d\n", currentPC, offset);)
        return 3;
    }

    BIH_REL:
    {
        const int8_t offset = GetNextByte();
        if(irqPin)
            PC += offset;
        LOG(fprintf(instructions, "%X\tBIH %d\n", currentPC, offset);)
        return 3;
    }

    // 0x3X
    NEG_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = -GetByte(addr);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        CCR[C] = data != 0;
        SetByte(addr, data);
        LOG(fprintf(instructions, "%X\tNEG 0x%X\n", currentPC, addr);)
        return 5;
    }

    COM_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = ~GetByte(addr);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        CCR[C] = true;
        LOG(fprintf(instructions, "%X\tCOM 0x%X\n", currentPC, addr);)
        return 5;
    }

    LSR_DIR:
    {
        const uint8_t addr = GetNextByte();
        uint8_t data = GetByte(addr);
        CCR[C] = (data & 1) ? true : false;
        data >>= 1;
        data &= 0x7F;
        CCR[N] = false;
        CCR[Z] = data == 0;
        SetByte(addr, data);
        LOG(fprintf(instructions, "%X\tLSR 0x%X\n", currentPC, addr);)
        return 5;
    }

    ROR_DIR:
    {
        const uint8_t addr = GetNextByte();
        uint8_t data = GetByte(addr);
        const uint8_t bit = CCR[C] << 7;
        CCR[C] = (data & 1) ? true : false;
        data >>= 1;
        data |= bit;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(addr, data);
        LOG(fprintf(instructions, "%X\tROR 0x%X\n", currentPC, addr);)
        return 5;
    }

    ASR_DIR:
    {
        const uint8_t addr = GetNextByte();
        uint8_t data = GetByte(addr);
        const uint8_t bit = (data & 0x80);
        CCR[C] = (data & 1) ? true : false;
        data >>= 1;
        data |= bit;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(addr, data);
        LOG(fprintf(instructions, "%X\tASR 0x%X\n", currentPC, addr);)
        return 5;
    }

    ASLLSL_DIR:
    {
        const uint8_t addr = GetNextByte();
        uint8_t data = GetByte(addr);
        CCR[C] = (data & 0x80) ? true : false;
        data <<= 1;
        CCR[N] = (data & 0x80);
        CCR[Z] = data == 0;
        SetByte(addr, data);
        LOG(fprintf(instructions, "%X\tASL/LSL 0x%X\n", currentPC, addr);)
        return 5;
    }

    ROL_DIR:
    {
        const uint8_t addr = GetNextByte();
        uint8_t data = GetByte(addr);
        const uint8_t bit = CCR[C];
        CCR[C] = (data & 0x80) ? true : false;
        data <<= 1;
        data |= bit;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(addr, data);
        LOG(fprintf(instructions, "%X\tROL 0x%X\n", currentPC, addr);)
        return 5;
    }

    DEC_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr) - 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(addr, data);
        LOG(fprintf(instructions, "%X\tDEC 0x%X\n", currentPC, addr);)
        return 5;
    }

    INC_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr) + 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(addr, data);
        LOG(fprintf(instructions, "%X\tINC 0x%X\n", currentPC, addr);)
        return 5;
    }

    TST_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        LOG(fprintf(instructions, "%X\tTST 0x%X\n", currentPC, addr);)
        return 4;
    }

    CLR_DIR:
    {
        const uint8_t addr = GetNextByte();
        CCR[N] = false;
        CCR[Z] = true;
        SetByte(addr, 0);
        LOG(fprintf(instructions, "%X\tCLR 0x%X\n", currentPC, addr);)
        return 5;
    }

    // 0x4X
    NEGA_INH:
    {
        A = -A;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        CCR[C] = A != 0;
        LOG(fprintf(instructions, "%X\tNEGA\n", currentPC);)
        return 3;
    }

    MUL_INH:
    {
        const uint16_t result = A * X;
        X = (result & 0xFF00) >> 8;
        A = result & 0xFF;
        CCR[H] = 0;
        CCR[C] = 0;
        LOG(fprintf(instructions, "%X\tMUL\n", currentPC);)
        return 1;
    }

    COMA_INH:
    {
        A = ~A;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        CCR[C] = true;
        LOG(fprintf(instructions, "%X\tCOMA\n", currentPC);)
        return 3;
    }

    LSRA_INH:
    {
        CCR[C] = (A & 1) ? true : false;
        A >>= 1;
        A &= 0x7F;
        CCR[N] = false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tLSRA\n", currentPC);)
        return 3;
    }

    RORA_INH:
    {
        const uint8_t bit = CCR[C] << 7;
        CCR[C] = (A & 1) ? true : false;
        A >>= 1;
        A |= bit;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tRORA\n", currentPC);)
        return 3;
    }

    ASRA_INH:
    {
        const uint8_t bit = (A & 0x80);
        CCR[C] = (A & 1) ? true : false;
        A >>= 1;
        A |= bit;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tASRA\n", currentPC);)
        return 3;
    }

    ASLALSLA_INH:
    {
        CCR[C] = (A & 0x80) ? true : false;
        A <<= 1;
        CCR[N] = (A & 0x80);
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tASLA/LSLA\n", currentPC);)
        return 3;
    }

    ROLA_INH:
    {
        const uint8_t bit = CCR[C];
        CCR[C] = (A & 0x80) ? true : false;
        A <<= 1;
        A |= bit;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tROLA\n", currentPC);)
        return 3;
    }

    DECA_INH:
    {
        A--;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tDECA\n", currentPC);)
        return 3;
    }

    INCA_INH:
    {
        A++;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tINCA\n", currentPC);)
        return 3;
    }

    TSTA_INH:
    {
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tTSTA\n", currentPC);)
        return 3;
    }

    CLRA_INH:
    {
        CCR[N] = false;
        CCR[Z] = true;
        A = 0;
        LOG(fprintf(instructions, "%X\tCLRA\n", currentPC);)
        return 3;
    }

    // 0x5X
    NEGX_INH:
    {
        X = -X;
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        CCR[C] = X != 0;
        LOG(fprintf(instructions, "%X\tNEGX\n", currentPC);)
        return 3;
    }

    COMX_INH:
    {
        X = ~X;
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        CCR[C] = true;
        LOG(fprintf(instructions, "%X\tCOMX\n", currentPC);)
        return 3;
    }

    LSRX_INH:
    {
        CCR[C] = (X & 1) ? true : false;
        X >>= 1;
        X &= 0x7F;
        CCR[N] = false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tLSRX\n", currentPC);)
        return 3;
    }

    RORX_INH:
    {
        const uint8_t bit = CCR[C] << 7;
        CCR[C] = (X & 1) ? true : false;
        X >>= 1;
        X |= bit;
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tRORX\n", currentPC);)
        return 3;
    }

    ASRX_INH:
    {
        const uint8_t bit = (X & 0x80);
        CCR[C] = (X & 1) ? true : false;
        X >>= 1;
        X |= bit;
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tASRX\n", currentPC);)
        return 3;
    }

    ASLXLSLX_INH:
    {
        CCR[C] = (X & 0x80) ? true : false;
        X <<= 1;
        CCR[N] = (X & 0x80);
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tASLX/LSLX\n", currentPC);)
        return 3;
    }

    ROLX_INH:
    {
        const uint8_t bit = CCR[C];
        CCR[C] = (X & 0x80) ? true : false;
        X <<= 1;
        X |= bit;
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tROLX\n", currentPC);)
        return 3;
    }

    DECX_INH:
    {
        X--;
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tDECX\n", currentPC);)
        return 3;
    }

    INCX_INH:
    {
        X++;
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tINCX\n", currentPC);)
        return 3;
    }

    TSTX_INH:
    {
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tTSTX\n", currentPC);)
        return 3;
    }

    CLRX_INH:
    {
        CCR[N] = false;
        CCR[Z] = true;
        X = 0;
        LOG(fprintf(instructions, "%X\tCLRX\n", currentPC);)
        return 3;
    }

    // 0x6X
    NEG_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = -GetByte(X + offset);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        CCR[C] = data != 0;
        SetByte(X + offset, data);
        LOG(fprintf(instructions, "%X\tNEG 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    COM_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = ~GetByte(X + offset);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        CCR[C] = true;
        LOG(fprintf(instructions, "%X\tCOM 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    LSR_IX1:
    {
        const uint8_t offset = GetNextByte();
        uint8_t data = GetByte(X + offset);
        CCR[C] = (data & 1) ? true : false;
        data >>= 1;
        data &= 0x7F;
        CCR[N] = false;
        CCR[Z] = data == 0;
        SetByte(X + offset, data);
        LOG(fprintf(instructions, "%X\tLSR 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    ROR_IX1:
    {
        const uint8_t offset = GetNextByte();
        uint8_t data = GetByte(X + offset);
        const uint8_t bit = CCR[C] << 7;
        CCR[C] = (data & 1) ? true : false;
        data >>= 1;
        data |= bit;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X + offset, data);
        LOG(fprintf(instructions, "%X\tROR 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    ASR_IX1:
    {
        const uint8_t offset = GetNextByte();
        uint8_t data = GetByte(X + offset);
        const uint8_t bit = (data & 0x80);
        CCR[C] = (data & 1) ? true : false;
        data >>= 1;
        data |= bit;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X + offset, data);
        LOG(fprintf(instructions, "%X\tASR 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    ASLLSL_IX1:
    {
        const uint8_t offset = GetNextByte();
        uint8_t data = GetByte(X + offset);
        CCR[C] = (data & 0x80) ? true : false;
        data <<= 1;
        CCR[N] = (data & 0x80);
        CCR[Z] = data == 0;
        SetByte(X + offset, data);
        LOG(fprintf(instructions, "%X\tASL/LSL 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    ROL_IX1:
    {
        const uint8_t offset = GetNextByte();
        uint8_t data = GetByte(X + offset);
        const uint8_t bit = CCR[C];
        CCR[C] = (data & 0x80) ? true : false;
        data <<= 1;
        data |= bit;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X + offset, data);
        LOG(fprintf(instructions, "%X\tROL 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    DEC_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset) - 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X + offset, data);
        LOG(fprintf(instructions, "%X\tDEC 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    INC_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset) + 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X + offset, data);
        LOG(fprintf(instructions, "%X\tINC 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    TST_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        LOG(fprintf(instructions, "%X\tTST 0x%X, X\n", currentPC, offset);)
        return 5;
    }

    CLR_IX1:
    {
        const uint8_t offset = GetNextByte();
        CCR[N] = false;
        CCR[Z] = true;
        SetByte(X + offset, 0);
        LOG(fprintf(instructions, "%X\tCLR 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    // 0x7X
    NEG_IX:
    {
        const uint8_t data = -GetByte(X);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        CCR[C] = data != 0;
        SetByte(X, data);
        LOG(fprintf(instructions, "%X\tNEG X\n", currentPC);)
        return 5;
    }

    COM_IX:
    {
        const uint8_t data = ~GetByte(X);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        CCR[C] = true;
        LOG(fprintf(instructions, "%X\tCOM X\n", currentPC);)
        return 5;
    }

    LSR_IX:
    {
        uint8_t data = GetByte(X);
        CCR[C] = (data & 1) ? true : false;
        data >>= 1;
        data &= 0x7F;
        CCR[N] = false;
        CCR[Z] = data == 0;
        SetByte(X, data);
        LOG(fprintf(instructions, "%X\tLSR X\n", currentPC);)
        return 5;
    }

    ROR_IX:
    {
        uint8_t data = GetByte(X);
        const uint8_t bit = CCR[C] << 7;
        CCR[C] = (data & 1) ? true : false;
        data >>= 1;
        data |= bit;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X, data);
        LOG(fprintf(instructions, "%X\tROR X\n", currentPC);)
        return 5;
    }

    ASR_IX:
    {
        uint8_t data = GetByte(X);
        const uint8_t bit = (data & 0x80);
        CCR[C] = (data & 1) ? true : false;
        data >>= 1;
        data |= bit;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X, data);
        LOG(fprintf(instructions, "%X\tASR X\n", currentPC);)
        return 5;
    }

    ASLLSL_IX:
    {
        uint8_t data = GetByte(X);
        CCR[C] = (data & 0x80) ? true : false;
        data <<= 1;
        CCR[N] = (data & 0x80);
        CCR[Z] = data == 0;
        SetByte(X, data);
        LOG(fprintf(instructions, "%X\tASL/LSL X\n", currentPC);)
        return 5;
    }

    ROL_IX:
    {
        uint8_t data = GetByte(X);
        const uint8_t bit = CCR[C];
        CCR[C] = (data & 0x80) ? true : false;
        data <<= 1;
        data |= bit;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X, data);
        LOG(fprintf(instructions, "%X\tROL X\n", currentPC);)
        return 5;
    }

    DEC_IX:
    {
        const uint8_t data = GetByte(X) - 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X, data);
        LOG(fprintf(instructions, "%X\tDEC X\n", currentPC);)
        return 5;
    }

    INC_IX:
    {
        const uint8_t data = GetByte(X) + 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X, data);
        LOG(fprintf(instructions, "%X\tINC X\n", currentPC);)
        return 5;
    }

    TST_IX:
    {
        const uint8_t data = GetByte(X);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        LOG(fprintf(instructions, "%X\tTST X\n", currentPC);)
        return 4;
    }

    CLR_IX:
    {
        CCR[N] = false;
        CCR[Z] = true;
        SetByte(X, 0);
        LOG(fprintf(instructions, "%X\tCLR X\n", currentPC);)
        return 5;
    }

    // 0x8X
    RTI_INH:
    {
        CCR = PopByte();
        A = PopByte();
        X = PopByte();
        PC = PopByte() << 8;
        PC |= PopByte();
        LOG(fprintf(instructions, "%X\tRTI\n", currentPC);)
        return 9;
    }

    RTS_INH:
    {
        PC = PopByte() << 8;
        PC |= PopByte();
        LOG(fprintf(instructions, "%X\tRTS\n", currentPC);)
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
        LOG(fprintf(instructions, "%X\tSWI\n", currentPC);)
        return 1 ? 0 : 0;
    }

    STOP_INH:
    {
        waitStop = true;
        pendingCycles = 2;
        CCR[I] = false;
        LOG(fprintf(instructions, "%X\tSTOP\n", currentPC);)
        return 2;
    }

    WAIT_INH:
    {
        waitStop = true;
        pendingCycles = 2;
        CCR[I] = false;
        LOG(fprintf(instructions, "%X\tWAIT\n", currentPC);)
        return 2;
    }

    // 0x9X
    TAX_INH:
        X = A;
        LOG(fprintf(instructions, "%X\tTAX\n", currentPC);)
        return 2;

    CLC_INH:
        CCR[C] = 0;
        LOG(fprintf(instructions, "%X\tCLC\n", currentPC);)
        return 2;

    SEC_INH:
        CCR[C] = 1;
        LOG(fprintf(instructions, "%X\tSEC\n", currentPC);)
        return 2;

    CLI_INH:
        CCR[I] = 0;
        LOG(fprintf(instructions, "%X\tCLI\n", currentPC);)
        return 2;

    SEI_INH:
        CCR[I] = 1;
        LOG(fprintf(instructions, "%X\tSEI\n", currentPC);)
        return 2;

    RSP_INH:
        SP = 0xFF;
        LOG(fprintf(instructions, "%X\tRSP\n", currentPC);)
        return 2;

    NOP_INH:
        LOG(fprintf(instructions, "%X\tNOP\n", currentPC);)
        return 2;

    TXA_INH:
        A = X;
        LOG(fprintf(instructions, "%X\tTAX\n", currentPC);)
        return 2;

    // 0xAX
    SUB_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSUB #0x%X\n", currentPC, data);)
        return 2;
    }

    CMP_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCMP #0x%X\n", currentPC, data);)
        return 2;
    }

    SBC_IMM:
    {
        const uint8_t data = GetNextByte() + CCR[C];
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSBC #0x%X\n", currentPC, data);)
        return 2;
    }

    CPX_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = X - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(X&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(X&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCPX #0x%X\n", currentPC, data);)
        return 2;
    }

    AND_IMM:
    {
        const uint8_t data = GetNextByte();
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tAND #0x%X\n", currentPC, data);)
        return 2;
    }

    BIT_IMM:
    {
        uint8_t data = GetNextByte();
        LOG(fprintf(instructions, "%X\tBIT #0x%X\n", currentPC, data);)
        data &= A;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        return 2;
    }

    LDA_IMM:
    {
        A = GetNextByte();
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tLDA #0x%X\n", currentPC, A);)
        return 2;
    }

    EOR_IMM:
    {
        const uint8_t data = GetNextByte();
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tEOR #0x%X\n", currentPC, data);)
        return 2;
    }

    ADC_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = A + data + CCR[C];
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (result & 0xFF00) ? true : false;
        LOG(fprintf(instructions, "%X\tADC #0x%X\n", currentPC, data);)
        A = result & 0x00FF;
        return 2;
    }

    ORA_IMM:
    {
        const uint8_t data = GetNextByte();
        A |= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tORA #0x%X\n", currentPC, data);)
        return 2;
    }

    ADD_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = A + data;
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (result & 0xFF00) ? true : false;
        LOG(fprintf(instructions, "%X\tADD #0x%X\n", currentPC, data);)
        A = result & 0x00FF;
        return 2;
    }

    BSR_REL:
    {
        int8_t offset = GetNextByte();
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC += offset;
        LOG(fprintf(instructions, "%X\tBSR %d\n", currentPC, offset);)
        return 6;
    }

    LDX_IMM:
    {
        X = GetNextByte();
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tLDX #0x%X\n", currentPC, X);)
        return 2;
    }

    // 0xBX
    SUB_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSUB 0x%X\n", currentPC, addr);)
        return 3;
    }

    CMP_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCMP 0x%X\n", currentPC, addr);)
        return 3;
    }

    SBC_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr) + CCR[C];
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSBC 0x%X\n", currentPC, addr);)
        return 3;
    }

    CPX_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        const uint16_t result = X - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(X&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(X&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCPX 0x%X\n", currentPC, addr);)
        return 3;
    }

    AND_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tAND 0x%X\n", currentPC, addr);)
        return 3;
    }

    BIT_DIR:
    {
        const uint8_t addr = GetNextByte();
        uint8_t data = GetByte(addr);
        LOG(fprintf(instructions, "%X\tBIT 0x%X\n", currentPC, addr);)
        data &= A;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        return 3;
    }

    LDA_DIR:
    {
        const uint8_t addr = GetNextByte();
        A = GetByte(addr);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tLDA 0x%X\n", currentPC, addr);)
        return 3;
    }

    STA_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, A);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tSTA 0x%X\n", currentPC, addr);)
        return 4;
    }

    EOR_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tEOR 0x%X\n", currentPC, addr);)
        return 3;
    }

    ADC_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        const uint16_t result = A + data + CCR[C];
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = ((A&0x80) && (data & 0x80)) || ((data&0x80) && !(result & 0x0080)) || ((A&0x80) && !(result & 0x0080));
        LOG(fprintf(instructions, "%X\tADC 0x%X\n", currentPC, addr);)
        A = result & 0x00FF;
        return 3;
    }

    ORA_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        A |= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tORA 0x%X\n", currentPC, addr);)
        return 3;
    }

    ADD_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        const uint16_t result = A + data;
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = ((A&0x80) && (data & 0x80)) || ((data&0x80) && !(result & 0x0080)) || ((A&0x80) && !(result & 0x0080));
        LOG(fprintf(instructions, "%X\tADD 0x%X\n", currentPC, addr);)
        A = result & 0x00FF;
        return 3;
    }

    JMP_DIR:
    {
        const uint8_t addr = GetNextByte();
        PC = addr;
        LOG(fprintf(instructions, "%X\tJMP 0x%X\n", currentPC, addr);)
        return 2;
    }

    JSR_DIR:
    {
        const uint8_t addr = GetNextByte();
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC = addr;
        LOG(fprintf(instructions, "%X\tJSR 0x%X\n", currentPC, addr);)
        return 5;
    }

    LDX_DIR:
    {
        const uint8_t addr = GetNextByte();
        X = GetByte(addr);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tLDX 0x%X\n", currentPC, addr);)
        return 3;
    }

    STX_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, X);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tSTX 0x%X\n", currentPC, addr);)
        return 4;
    }

    // 0xCX
    SUB_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(addr);
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSUB 0x%X\n", currentPC, addr);)
        return 4;
    }

    CMP_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(addr);
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCMP 0x%X\n", currentPC, addr);)
        return 4;
    }

    SBC_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(addr) + CCR[C];
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSBC 0x%X\n", currentPC, addr);)
        return 4;
    }

    CPX_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(addr);
        const uint16_t result = X - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(X&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(X&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCPX 0x%X\n", currentPC, addr);)
        return 4;
    }

    AND_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(addr);
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tAND 0x%X\n", currentPC, addr);)
        return 4;
    }

    BIT_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        uint8_t data = GetByte(addr);
        LOG(fprintf(instructions, "%X\tBIT 0x%X\n", currentPC, addr);)
        data &= A;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        return 4;
    }

    LDA_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        A = GetByte(addr);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tLDA 0x%X\n", currentPC, addr);)
        return 4;
    }

    STA_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        SetByte(addr, A);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tSTA 0x%X\n", currentPC, addr);)
        return 5;
    }

    EOR_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(addr);
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tEOR 0x%X\n", currentPC, addr);)
        return 4;
    }

    ADC_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(addr);
        const uint16_t result = A + data + CCR[C];
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = ((A&0x80) && (data & 0x80)) || ((data&0x80) && !(result & 0x0080)) || ((A&0x80) && !(result & 0x0080));
        LOG(fprintf(instructions, "%X\tADC 0x%X\n", currentPC, addr);)
        A = result & 0x00FF;
        return 4;
    }

    ORA_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(addr);
        A |= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tORA 0x%X\n", currentPC, addr);)
        return 4;
    }

    ADD_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(addr);
        const uint16_t result = A + data;
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = ((A&0x80) && (data & 0x80)) || ((data&0x80) && !(result & 0x0080)) || ((A&0x80) && !(result & 0x0080));
        LOG(fprintf(instructions, "%X\tADD 0x%X\n", currentPC, addr);)
        A = result & 0x00FF;
        return 4;
    }

    JMP_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        PC = addr;
        LOG(fprintf(instructions, "%X\tJMP 0x%X\n", currentPC, addr);)
        return 3;
    }

    JSR_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC = addr;
        LOG(fprintf(instructions, "%X\tJSR 0x%X\n", currentPC, addr);)
        return 6;
    }

    LDX_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        X = GetByte(addr);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tLDX 0x%X\n", currentPC, addr);)
        return 4;
    }

    STX_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        SetByte(addr, X);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tSTX 0x%X\n", currentPC, addr);)
        return 5;
    }

    // 0xDX
    SUB_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(X + offset);
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSUB 0x%X, X\n", currentPC, offset);)
        return 5;
    }

    CMP_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(X + offset);
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCMP 0x%X, X\n", currentPC, offset);)
        return 5;
    }

    SBC_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(X + offset) + CCR[C];
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSBC 0x%X, X\n", currentPC, offset);)
        return 5;
    }

    CPX_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(X + offset);
        const uint16_t result = X - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(X&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(X&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCPX 0x%X, X\n", currentPC, offset);)
        return 5;
    }

    AND_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(X + offset);
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tAND 0x%X, X\n", currentPC, offset);)
        return 5;
    }

    BIT_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        uint8_t data = GetByte(X + offset);
        LOG(fprintf(instructions, "%X\tBIT 0x%X, X\n", currentPC, offset);)
        data &= A;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        return 5;
    }

    LDA_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t A = GetByte(X + offset);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tLDA 0x%X, X\n", currentPC, offset);)
        return 5;
    }

    STA_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        SetByte(X + offset, A);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tSTA 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    EOR_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(X + offset);
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tEOR 0x%X, X\n", currentPC, offset);)
        return 5;
    }

    ADC_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(X + offset);
        const uint16_t result = A + data + CCR[C];
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = ((A&0x80) && (data & 0x80)) || ((data&0x80) && !(result & 0x0080)) || ((A&0x80) && !(result & 0x0080));
        LOG(fprintf(instructions, "%X\tADC 0x%X, X\n", currentPC, offset);)
        A = result & 0x00FF;
        return 5;
    }

    ORA_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(X + offset);
        A |= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tORA 0x%X, X\n", currentPC, offset);)
        return 5;
    }

    ADD_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(X + offset);
        const uint16_t result = A + data;
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = ((A&0x80) && (data & 0x80)) || ((data&0x80) && !(result & 0x0080)) || ((A&0x80) && !(result & 0x0080));
        LOG(fprintf(instructions, "%X\tADD 0x%X, X\n", currentPC, offset);)
        A = result & 0x00FF;
        return 5;
    }

    JMP_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        PC = X + offset;
        LOG(fprintf(instructions, "%X\tJMP 0x%X, X\n", currentPC, offset);)
        return 4;
    }

    JSR_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC = X + offset;
        LOG(fprintf(instructions, "%X\tJSR 0x%X, X\n", currentPC, offset);)
        return 7;
    }

    LDX_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        LOG(fprintf(instructions, "%X\tLDX 0x%X, X\n", currentPC, offset);)
        X = GetByte(X + offset);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        return 5;
    }

    STX_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        SetByte(X + offset, X);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tSTX 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    // 0xEX
    SUB_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSUB 0x%X, X\n", currentPC, offset);)
        return 4;
    }

    CMP_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCMP 0x%X, X\n", currentPC, offset);)
        return 4;
    }

    SBC_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset) + CCR[C];
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSBC 0x%X, X\n", currentPC, offset);)
        return 4;
    }

    CPX_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        const uint16_t result = X - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(X&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(X&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCPX 0x%X, X\n", currentPC, offset);)
        return 4;
    }

    AND_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tAND 0x%X, X\n", currentPC, offset);)
        return 4;
    }

    BIT_IX1:
    {
        const uint8_t offset = GetNextByte();
        uint8_t data = GetByte(X + offset);
        LOG(fprintf(instructions, "%X\tBIT 0x%X, X\n", currentPC, offset);)
        data &= A;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        return 4;
    }

    LDA_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t A = GetByte(X + offset);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tLDA 0x%X, X\n", currentPC, offset);)
        return 4;
    }

    STA_IX1:
    {
        const uint8_t offset = GetNextByte();
        SetByte(X + offset, A);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tSTA 0x%X, X\n", currentPC, offset);)
        return 5;
    }

    EOR_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tEOR 0x%X, X\n", currentPC, offset);)
        return 4;
    }

    ADC_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        const uint16_t result = A + data + CCR[C];
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = ((A&0x80) && (data & 0x80)) || ((data&0x80) && !(result & 0x0080)) || ((A&0x80) && !(result & 0x0080));
        LOG(fprintf(instructions, "%X\tADC 0x%X, X\n", currentPC, offset);)
        A = result & 0x00FF;
        return 4;
    }

    ORA_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        A |= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tORA 0x%X, X\n", currentPC, offset);)
        return 4;
    }

    ADD_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        const uint16_t result = A + data;
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = ((A&0x80) && (data & 0x80)) || ((data&0x80) && !(result & 0x0080)) || ((A&0x80) && !(result & 0x0080));
        LOG(fprintf(instructions, "%X\tADD 0x%X, X\n", currentPC, offset);)
        A = result & 0x00FF;
        return 4;
    }

    JMP_IX1:
    {
        const uint8_t offset = GetNextByte();
        PC = X + offset;
        LOG(fprintf(instructions, "%X\tJMP 0x%X, X\n", currentPC, offset);)
        return 3;
    }

    JSR_IX1:
    {
        const uint8_t offset = GetNextByte();
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC = X + offset;
        LOG(fprintf(instructions, "%X\tJSR 0x%X, X\n", currentPC, offset);)
        return 6;
    }

    LDX_IX1:
    {
        const uint8_t offset = GetNextByte();
        LOG(fprintf(instructions, "%X\tLDX 0x%X, X\n", currentPC, offset);)
        X = GetByte(X + offset);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        return 4;
    }

    STX_IX1:
    {
        const uint8_t offset = GetNextByte();
        SetByte(X + offset, X);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tSTX 0x%X, X\n", currentPC, offset);)
        return 5;
    }

    // 0xFX
    SUB_IX:
    {
        const uint8_t data = GetByte(X);
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSUB X\n", currentPC);)
        return 3;
    }

    CMP_IX:
    {
        const uint8_t data = GetByte(X);
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCMP X\n", currentPC);)
        return 3;
    }

    SBC_IX:
    {
        const uint8_t data = GetByte(X) + CCR[C];
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(fprintf(instructions, "%X\tSBC X\n", currentPC);)
        return 3;
    }

    CPX_IX:
    {
        const uint8_t data = GetByte(X);
        const uint16_t result = X - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(X&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(X&0x80) && (result & 0x0080));
        LOG(fprintf(instructions, "%X\tCPX X\n", currentPC);)
        return 3;
    }

    AND_IX:
    {
        const uint8_t data = GetByte(X);
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tAND X\n", currentPC);)
        return 3;
    }

    BIT_IX:
    {
        uint8_t data = GetByte(X);
        LOG(fprintf(instructions, "%X\tBIT X\n", currentPC);)
        data &= A;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        return 3;
    }

    LDA_IX:
    {
        A = GetByte(X);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tLDA X\n", currentPC);)
        return 3;
    }

    STA_IX:
    {
        SetByte(X, A);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tSTA X\n", currentPC);)
        return 4;
    }

    EOR_IX:
    {
        const uint8_t data = GetByte(X);
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tEOR X\n", currentPC);)
        return 3;
    }

    ADC_IX:
    {
        const uint8_t data = GetByte(X);
        const uint16_t result = A + data + CCR[C];
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = ((A&0x80) && (data & 0x80)) || ((data&0x80) && !(result & 0x0080)) || ((A&0x80) && !(result & 0x0080));
        LOG(fprintf(instructions, "%X\tADC X\n", currentPC);)
        A = result & 0x00FF;
        return 3;
    }

    ORA_IX:
    {
        const uint8_t data = GetByte(X);
        A |= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(fprintf(instructions, "%X\tORA X\n", currentPC);)
        return 3;
    }

    ADD_IX:
    {
        const uint8_t data = GetByte(X);
        const uint16_t result = A + data;
        CCR[H] = ((A&0x8) && (data&0x8)) || ((data&0x8) && !(result&0x8)) || (!(result&0x8) && (A&0x8));
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = ((A&0x80) && (data & 0x80)) || ((data&0x80) && !(result & 0x0080)) || ((A&0x80) && !(result & 0x0080));
        LOG(fprintf(instructions, "%X\tADD X\n", currentPC);)
        A = result & 0x00FF;
        return 3;
    }

    JMP_IX:
    {
        PC = X;
        LOG(fprintf(instructions, "%X\tJMP X\n", currentPC);)
        return 2;
    }

    JSR_IX:
    {
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC = X;
        LOG(fprintf(instructions, "%X\tJSR X\n", currentPC);)
        return 5;
    }

    LDX_IX:
    {
        LOG(fprintf(instructions, "%X\tLDX X\n", currentPC);)
        X = GetByte(X);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        return 3;
    }

    STX_IX:
    {
        SetByte(X, X);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(fprintf(instructions, "%X\tSTX X\n", currentPC);)
        return 4;
    }

    unknown:
        LOG(fprintf(instructions, "%X\tUnknwon instruction: 0x%X\n", currentPC, currentOpcode);)
    return 0;
}
