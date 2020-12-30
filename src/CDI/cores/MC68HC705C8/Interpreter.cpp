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
        LOG(instructions << std::hex << currentPC << "\tBRSET0 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRCLR0 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRSET1 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRCLR1 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRSET2 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRCLR2 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRSET3 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRCLR3 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRSET4 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRCLR4 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRSET5 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRCLR5 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRSET6 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRCLR6 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRSET7 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tBRCLR7 0x" << (uint16_t)addr << ", " << std::dec << (uint16_t)offset << std::endl)
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
    {
        const int8_t offset = GetNextByte();
        PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBRA " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BRN_REL:
    {
        const int8_t offset = GetNextByte();
        LOG(instructions << std::hex << currentPC << "\tBRN " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BHI_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[C] && !CCR[Z])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBHI " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BLS_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[C] || CCR[Z])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBLS " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BCC_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[C])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBCC " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BCSBLO_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[C])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBLO/BCS " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BNE_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[Z])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBNE " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BEQ_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[Z])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBEQ " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BHCC_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[H])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBHCC " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BHCS_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[H])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBHCS " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BPL_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[N])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBPL " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BMI_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[N])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBMI " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BMC_REL:
    {
        const int8_t offset = GetNextByte();
        if(!CCR[I])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBMC " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BMS_REL:
    {
        const int8_t offset = GetNextByte();
        if(CCR[I])
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBMS " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BIL_REL:
    {
        const int8_t offset = GetNextByte();
        if(!irqPin)
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBIL " << std::dec << (int16_t)offset << std::endl)
        return 3;
    }

    BIH_REL:
    {
        const int8_t offset = GetNextByte();
        if(irqPin)
            PC += offset;
        LOG(instructions << std::hex << currentPC << "\tBIH " << std::dec << (int16_t)offset << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tNEG 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    COM_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = ~GetByte(addr);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        CCR[C] = true;
        LOG(instructions << std::hex << currentPC << "\tCOM 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tLSR 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tROR 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tASR 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tASL/LSL 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tROL 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    DEC_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr) - 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(addr, data);
        LOG(instructions << std::hex << currentPC << "\tDEC 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    INC_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr) + 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(addr, data);
        LOG(instructions << std::hex << currentPC << "\tINC 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    TST_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        LOG(instructions << std::hex << currentPC << "\tTST 0x" << (uint16_t)addr << std::endl)
        return 4;
    }

    CLR_DIR:
    {
        const uint8_t addr = GetNextByte();
        CCR[N] = false;
        CCR[Z] = true;
        SetByte(addr, 0);
        LOG(instructions << std::hex << currentPC << "\tCLR 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    // 0x4X
    NEGA_INH:
    {
        A = -A;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        CCR[C] = A != 0;
        LOG(instructions << std::hex << currentPC << "\tNEGA" << std::endl)
        return 3;
    }

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
    {
        A = ~A;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        CCR[C] = true;
        LOG(instructions << std::hex << currentPC << "\tCOMA" << std::endl)
        return 3;
    }

    LSRA_INH:
    {
        CCR[C] = (A & 1) ? true : false;
        A >>= 1;
        A &= 0x7F;
        CCR[N] = false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tLSRA" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tRORA" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tASRA" << std::endl)
        return 3;
    }

    ASLALSLA_INH:
    {
        CCR[C] = (A & 0x80) ? true : false;
        A <<= 1;
        CCR[N] = (A & 0x80);
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tASLA/LSLA" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tROLA" << std::endl)
        return 3;
    }

    DECA_INH:
    {
        A--;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tDECA" << std::endl)
        return 3;
    }

    INCA_INH:
    {
        A++;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tINCA" << std::endl)
        return 3;
    }

    TSTA_INH:
    {
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tTSTA" << std::endl)
        return 3;
    }

    CLRA_INH:
    {
        CCR[N] = false;
        CCR[Z] = true;
        A = 0;
        LOG(instructions << std::hex << currentPC << "\tCLRA" << std::endl)
        return 3;
    }

    // 0x5X
    NEGX_INH:
    {
        X = -X;
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        CCR[C] = X != 0;
        LOG(instructions << std::hex << currentPC << "\tNEGX" << std::endl)
        return 3;
    }

    COMX_INH:
    {
        X = ~X;
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        CCR[C] = true;
        LOG(instructions << std::hex << currentPC << "\tCOMX" << std::endl)
        return 3;
    }

    LSRX_INH:
    {
        CCR[C] = (X & 1) ? true : false;
        X >>= 1;
        X &= 0x7F;
        CCR[N] = false;
        CCR[Z] = X == 0;
        LOG(instructions << std::hex << currentPC << "\tLSRX" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tRORX" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tASRX" << std::endl)
        return 3;
    }

    ASLXLSLX_INH:
    {
        CCR[C] = (X & 0x80) ? true : false;
        X <<= 1;
        CCR[N] = (X & 0x80);
        CCR[Z] = X == 0;
        LOG(instructions << std::hex << currentPC << "\tASLX/LSLX" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tROLX" << std::endl)
        return 3;
    }

    DECX_INH:
    {
        X--;
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(instructions << std::hex << currentPC << "\tDECX" << std::endl)
        return 3;
    }

    INCX_INH:
    {
        X++;
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(instructions << std::hex << currentPC << "\tINCX" << std::endl)
        return 3;
    }

    TSTX_INH:
    {
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(instructions << std::hex << currentPC << "\tTSTX" << std::endl)
        return 3;
    }

    CLRX_INH:
    {
        CCR[N] = false;
        CCR[Z] = true;
        X = 0;
        LOG(instructions << std::hex << currentPC << "\tCLRX" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tNEG 0x" << (uint16_t)offset << ", X" << std::endl)
        return 6;
    }

    COM_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = ~GetByte(X + offset);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        CCR[C] = true;
        LOG(instructions << std::hex << currentPC << "\tCOM 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tLSR 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tROR 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tASR 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tASL/LSL 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tROL 0x" << (uint16_t)offset << ", X" << std::endl)
        return 6;
    }

    DEC_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset) - 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X + offset, data);
        LOG(instructions << std::hex << currentPC << "\tDEC 0x" << (uint16_t)offset << ", X" << std::endl)
        return 6;
    }

    INC_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset) + 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X + offset, data);
        LOG(instructions << std::hex << currentPC << "\tINC 0x" << (uint16_t)offset << ", X" << std::endl)
        return 6;
    }

    TST_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        LOG(instructions << std::hex << currentPC << "\tTST 0x" << (uint16_t)offset << ", X" << std::endl)
        return 5;
    }

    CLR_IX1:
    {
        const uint8_t offset = GetNextByte();
        CCR[N] = false;
        CCR[Z] = true;
        SetByte(X + offset, 0);
        LOG(instructions << std::hex << currentPC << "\tCLR 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tNEG X" << std::endl)
        return 5;
    }

    COM_IX:
    {
        const uint8_t data = ~GetByte(X);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        CCR[C] = true;
        LOG(instructions << std::hex << currentPC << "\tCOM X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tLSR X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tROR X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tASR X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tASL/LSL X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tROL X" << std::endl)
        return 5;
    }

    DEC_IX:
    {
        const uint8_t data = GetByte(X) - 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X, data);
        LOG(instructions << std::hex << currentPC << "\tDEC X" << std::endl)
        return 5;
    }

    INC_IX:
    {
        const uint8_t data = GetByte(X) + 1;
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        SetByte(X, data);
        LOG(instructions << std::hex << currentPC << "\tINC X" << std::endl)
        return 5;
    }

    TST_IX:
    {
        const uint8_t data = GetByte(X);
        CCR[N] = (data & 0x80) ? true : false;
        CCR[Z] = data == 0;
        LOG(instructions << std::hex << currentPC << "\tTST X" << std::endl)
        return 4;
    }

    CLR_IX:
    {
        CCR[N] = false;
        CCR[Z] = true;
        SetByte(X, 0);
        LOG(instructions << std::hex << currentPC << "\tCLR X" << std::endl)
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
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        A = result & 0x00FF;
        LOG(instructions << std::hex << currentPC << "\tSUB #0x" << (uint16_t)data << std::endl)
        return 2;
    }

    CMP_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        LOG(instructions << std::hex << currentPC << "\tCMP #0x" << (uint16_t)data << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSBC #0x" << (uint16_t)data << std::endl)
        return 2;
    }

    CPX_IMM:
    {
        const uint8_t data = GetNextByte();
        const uint16_t result = X - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(X&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(X&0x80) && (result & 0x0080));
        LOG(instructions << std::hex << currentPC << "\tCPX #0x" << (uint16_t)data << std::endl)
        return 2;
    }

    AND_IMM:
    {
        const uint8_t data = GetNextByte();
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tAND #0x" << (uint16_t)data << std::endl)
        return 2;
    }

    BIT_IMM:
    {
        uint8_t data = GetNextByte();
        LOG(instructions << std::hex << currentPC << "\tBIT #0x" << (uint16_t)data << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tLDA #0x" << (uint16_t)A << std::endl)
        return 2;
    }

    EOR_IMM:
    {
        const uint8_t data = GetNextByte();
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tEOR #0x" << (uint16_t)data << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tADC #0x" << (uint16_t)data << std::endl)
        A = result & 0x00FF;
        return 2;
    }

    ORA_IMM:
    {
        const uint8_t data = GetNextByte();
        A |= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tORA #0x" << (uint16_t)data << std::endl)
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
        CCR[Z] = X == 0;
        LOG(instructions << std::hex << currentPC << "\tLDX #0x" << (uint16_t)X << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSUB 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tCMP 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSBC 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tCPX 0x" << (uint16_t)addr << std::endl)
        return 3;
    }

    AND_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tAND 0x" << (uint16_t)addr << std::endl)
        return 3;
    }

    BIT_DIR:
    {
        const uint8_t addr = GetNextByte();
        uint8_t data = GetByte(addr);
        LOG(instructions << std::hex << currentPC << "\tBIT 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tLDA 0x" << (uint16_t)addr << std::endl)
        return 3;
    }

    STA_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, A);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tSTA 0x" << (uint16_t)addr << std::endl)
        return 4;
    }

    EOR_DIR:
    {
        const uint8_t addr = GetNextByte();
        const uint8_t data = GetByte(addr);
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tEOR 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tADC 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tORA 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tADD 0x" << (uint16_t)addr << std::endl)
        A = result & 0x00FF;
        return 3;
    }

    JMP_DIR:
    {
        const uint8_t addr = GetNextByte();
        PC = addr;
        LOG(instructions << std::hex << currentPC << "\tJMP 0x" << (uint16_t)addr << std::endl)
        return 2;
    }

    JSR_DIR:
    {
        const uint8_t addr = GetNextByte();
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC = addr;
        LOG(instructions << std::hex << currentPC << "\tJSR 0x" << (uint16_t)addr << std::endl)
        return 5;
    }

    LDX_DIR:
    {
        const uint8_t addr = GetNextByte();
        X = GetByte(addr);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(instructions << std::hex << currentPC << "\tLDX 0x" << (uint16_t)addr << std::endl)
        return 3;
    }

    STX_DIR:
    {
        const uint8_t addr = GetNextByte();
        SetByte(addr, X);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(instructions << std::hex << currentPC << "\tSTX 0x" << (uint16_t)addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSUB 0x" << addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tCMP 0x" << addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSBC 0x" << addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tCPX 0x" << addr << std::endl)
        return 4;
    }

    AND_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(addr);
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tAND 0x" << addr << std::endl)
        return 4;
    }

    BIT_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        uint8_t data = GetByte(addr);
        LOG(instructions << std::hex << currentPC << "\tBIT 0x" << addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tLDA 0x" << addr << std::endl)
        return 4;
    }

    STA_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        SetByte(addr, A);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tSTA 0x" << addr << std::endl)
        return 5;
    }

    EOR_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(addr);
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tEOR 0x" << addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tADC 0x" << addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tORA 0x" << addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tADD 0x" << addr << std::endl)
        A = result & 0x00FF;
        return 4;
    }

    JMP_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        PC = addr;
        LOG(instructions << std::hex << currentPC << "\tJMP 0x" << addr << std::endl)
        return 3;
    }

    JSR_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC = addr;
        LOG(instructions << std::hex << currentPC << "\tJSR 0x" << addr << std::endl)
        return 6;
    }

    LDX_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        X = GetByte(addr);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(instructions << std::hex << currentPC << "\tLDX 0x" << addr << std::endl)
        return 4;
    }

    STX_EXT:
    {
        const uint16_t addr = GetNextByte() << 8 | GetNextByte();
        SetByte(addr, X);
        CCR[N] = (X & 0x80) ? true : false;
        CCR[Z] = X == 0;
        LOG(instructions << std::hex << currentPC << "\tSTX 0x" << addr << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSUB 0x" << offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tCMP 0x" << offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSBC 0x" << offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tCPX 0x" << offset << ", X" << std::endl)
        return 5;
    }

    AND_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(X + offset);
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tAND 0x" << offset << ", X" << std::endl)
        return 5;
    }

    BIT_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        uint8_t data = GetByte(X + offset);
        LOG(instructions << std::hex << currentPC << "\tBIT 0x" << offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tLDA 0x" << offset << ", X" << std::endl)
        return 5;
    }

    STA_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        SetByte(X + offset, A);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tSTA 0x" << offset << ", X" << std::endl)
        return 6;
    }

    EOR_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        const uint8_t data = GetByte(X + offset);
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tEOR 0x" << offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tADC 0x" << offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tORA 0x" << offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tADD 0x" << offset << ", X" << std::endl)
        A = result & 0x00FF;
        return 5;
    }

    JMP_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        PC = X + offset;
        LOG(instructions << std::hex << currentPC << "\tJMP 0x" << offset << ", X" << std::endl)
        return 4;
    }

    JSR_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC = X + offset;
        LOG(instructions << std::hex << currentPC << "\tJSR 0x" << offset << ", X" << std::endl)
        return 7;
    }

    LDX_IX2:
    {
        const uint16_t offset = GetNextByte() << 8 | GetNextByte();
        LOG(instructions << std::hex << currentPC << "\tLDX 0x" << offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSTX 0x" << offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSUB 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tCMP 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSBC 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tCPX 0x" << (uint16_t)offset << ", X" << std::endl)
        return 4;
    }

    AND_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tAND 0x" << (uint16_t)offset << ", X" << std::endl)
        return 4;
    }

    BIT_IX1:
    {
        const uint8_t offset = GetNextByte();
        uint8_t data = GetByte(X + offset);
        LOG(instructions << std::hex << currentPC << "\tBIT 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tLDA 0x" << (uint16_t)offset << ", X" << std::endl)
        return 4;
    }

    STA_IX1:
    {
        const uint8_t offset = GetNextByte();
        SetByte(X + offset, A);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tSTA 0x" << (uint16_t)offset << ", X" << std::endl)
        return 5;
    }

    EOR_IX1:
    {
        const uint8_t offset = GetNextByte();
        const uint8_t data = GetByte(X + offset);
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tEOR 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tADC 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tORA 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tADD 0x" << (uint16_t)offset << ", X" << std::endl)
        A = result & 0x00FF;
        return 4;
    }

    JMP_IX1:
    {
        const uint8_t offset = GetNextByte();
        PC = X + offset;
        LOG(instructions << std::hex << currentPC << "\tJMP 0x" << (uint16_t)offset << ", X" << std::endl)
        return 3;
    }

    JSR_IX1:
    {
        const uint8_t offset = GetNextByte();
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC = X + offset;
        LOG(instructions << std::hex << currentPC << "\tJSR 0x" << (uint16_t)offset << ", X" << std::endl)
        return 6;
    }

    LDX_IX1:
    {
        const uint8_t offset = GetNextByte();
        LOG(instructions << std::hex << currentPC << "\tLDX 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSTX 0x" << (uint16_t)offset << ", X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSUB X" << std::endl)
        return 3;
    }

    CMP_IX:
    {
        const uint8_t data = GetByte(X);
        const uint16_t result = A - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(A&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(A&0x80) && (result & 0x0080));
        LOG(instructions << std::hex << currentPC << "\tCMP X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSBC X" << std::endl)
        return 3;
    }

    CPX_IX:
    {
        const uint8_t data = GetByte(X);
        const uint16_t result = X - data;
        CCR[N] = (result & 0x0080) ? true : false;
        CCR[Z] = result == 0;
        CCR[C] = (!(X&0x80) && (data & 0x80)) || ((data&0x80) && (result & 0x0080)) || (!(X&0x80) && (result & 0x0080));
        LOG(instructions << std::hex << currentPC << "\tCPX X" << std::endl)
        return 3;
    }

    AND_IX:
    {
        const uint8_t data = GetByte(X);
        A &= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tAND X" << std::endl)
        return 3;
    }

    BIT_IX:
    {
        uint8_t data = GetByte(X);
        LOG(instructions << std::hex << currentPC << "\tBIT X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tLDA X" << std::endl)
        return 3;
    }

    STA_IX:
    {
        SetByte(X, A);
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tSTA X" << std::endl)
        return 4;
    }

    EOR_IX:
    {
        const uint8_t data = GetByte(X);
        A ^= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tEOR X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tADC X" << std::endl)
        A = result & 0x00FF;
        return 3;
    }

    ORA_IX:
    {
        const uint8_t data = GetByte(X);
        A |= data;
        CCR[N] = (A & 0x80) ? true : false;
        CCR[Z] = A == 0;
        LOG(instructions << std::hex << currentPC << "\tORA X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tADD X" << std::endl)
        A = result & 0x00FF;
        return 3;
    }

    JMP_IX:
    {
        PC = X;
        LOG(instructions << std::hex << currentPC << "\tJMP X" << std::endl)
        return 2;
    }

    JSR_IX:
    {
        PushByte(PC & 0x00FF);
        PushByte(PC >> 8);
        PC = X;
        LOG(instructions << std::hex << currentPC << "\tJSR X" << std::endl)
        return 5;
    }

    LDX_IX:
    {
        LOG(instructions << std::hex << currentPC << "\tLDX X" << std::endl)
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
        LOG(instructions << std::hex << currentPC << "\tSTX X" << std::endl)
        return 4;
    }

    unknown:
        LOG(instructions << std::hex << currentPC << "\tUnknwon instruction: 0x" << (uint16_t)currentOpcode << std::endl)
    return 0;
}
