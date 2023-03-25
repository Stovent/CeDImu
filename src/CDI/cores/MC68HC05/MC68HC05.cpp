#include "MC68HC05.hpp"

/** \brief Triggers the RESET pin.
 *
 * Default implementation fetches the PC from the RESET vector and sets \ref MC68HC05.wait and \ref MC68HC05.stop to false.
 */
void MC68HC05::Reset()
{
    PC = (uint16_t)GetMemory(memorySize - 2) << 8;
    PC |= GetMemory(memorySize - 1);
    stop = wait = false;
}

/** \brief Executes a single instruction.
 * \return The number of cycles used to execute the instruction. 0 if in WAIT or STOP mode.
 */
size_t MC68HC05::Interpreter()
{
    if(wait || stop)
        return 0;

    static constexpr void* ITC[256] = {
        &&BRSET0_DIR, &&BRCLR0_DIR, &&BRSET1_DIR, &&BRCLR1_DIR, &&BRSET2_DIR, &&BRCLR2_DIR, &&BRSET3_DIR, &&BRCLR3_DIR, &&BRSET4_DIR, &&BRCLR4_DIR, &&BRSET5_DIR, &&BRCLR5_DIR, &&BRSET6_DIR, &&BRCLR6_DIR, &&BRSET7_DIR, &&BRCLR7_DIR, // 0x0X
        &&BSET0_DIR,  &&BCLR0_DIR,  &&BSET1_DIR,  &&BCLR1_DIR,  &&BSET2_DIR,  &&BCLR2_DIR,  &&BSET3_DIR,  &&BCLR3_DIR,  &&BSET4_DIR,  &&BCLR4_DIR,  &&BSET5_DIR,  &&BCLR5_DIR,  &&BSET6_DIR,  &&BCLR6_DIR,  &&BSET7_DIR,  &&BCLR7_DIR,  // 0x1X
        &&BRA_REL,    &&BRN_REL,    &&BHI_REL,    &&BLS_REL,    &&BCC_REL,    &&BCS_REL,    &&BNE_REL,    &&BEQ_REL,    &&BHCC_REL,   &&BHCS_REL,   &&BPL_REL,    &&BMI_REL,    &&BMC_REL,    &&BMS_REL,    &&BIL_REL,    &&BIH_REL,    // 0x2X
        &&NEG_DIR,    &&unknown,    &&unknown,    &&COM_DIR,    &&LSR_DIR,    &&unknown,    &&ROR_DIR,    &&ASR_DIR,    &&LSL_DIR,    &&ROL_DIR,    &&DEC_DIR,    &&unknown,    &&INC_DIR,    &&TST_DIR,    &&unknown,    &&CLR_DIR,    // 0x3X
        &&NEGA_INH,   &&unknown,    &&MUL_INH,    &&COMA_INH,   &&LSRA_INH,   &&unknown,    &&RORA_INH,   &&ASRA_INH,   &&LSLA_INH,   &&ROLA_INH,   &&DECA_INH,   &&unknown,    &&INCA_INH,   &&TSTA_INH,   &&unknown,    &&CLRA_INH,   // 0x4X
        &&NEGX_INH,   &&unknown,    &&unknown,    &&COMX_INH,   &&LSRX_INH,   &&unknown,    &&RORX_INH,   &&ASRX_INH,   &&LSLX_INH,   &&ROLX_INH,   &&DECX_INH,   &&unknown,    &&INCX_INH,   &&TSTX_INH,   &&unknown,    &&CLRX_INH,   // 0x5X
        &&NEG_IX1,    &&unknown,    &&unknown,    &&COM_IX1,    &&LSR_IX1,    &&unknown,    &&ROR_IX1,    &&ASR_IX1,    &&LSL_IX1,    &&ROL_IX1,    &&DEC_IX1,    &&unknown,    &&INC_IX1,    &&TST_IX1,    &&unknown,    &&CLR_IX1,    // 0x6X
        &&NEG_IX,     &&unknown,    &&unknown,    &&COM_IX,     &&LSR_IX,     &&unknown,    &&ROR_IX,     &&ASR_IX,     &&LSL_IX,     &&ROL_IX,     &&DEC_IX,     &&unknown,    &&INC_IX,     &&TST_IX,     &&unknown,    &&CLR_IX,     // 0x7X
        &&RTI_INH,    &&RTS_INH,    &&unknown,    &&SWI_INH,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&STOP_INH,   &&WAIT_INH,   // 0x8X
        &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&unknown,    &&TAX_INH,    &&CLC_INH,    &&SEC_INH,    &&CLI_INH,    &&SEI_INH,    &&RSP_INH,    &&NOP_INH,    &&unknown,    &&TXA_INH,    // 0x9X
        &&SUB_IMM,    &&CMP_IMM,    &&SBC_IMM,    &&CPX_IMM,    &&AND_IMM,    &&BIT_IMM,    &&LDA_IMM,    &&unknown,    &&EOR_IMM,    &&ADC_IMM,    &&ORA_IMM,    &&ADD_IMM,    &&unknown,    &&BSR_REL,    &&LDX_IMM,    &&unknown,    // 0xAX
        &&SUB_DIR,    &&CMP_DIR,    &&SBC_DIR,    &&CPX_DIR,    &&AND_DIR,    &&BIT_DIR,    &&LDA_DIR,    &&STA_DIR,    &&EOR_DIR,    &&ADC_DIR,    &&ORA_DIR,    &&ADD_DIR,    &&JMP_DIR,    &&JSR_DIR,    &&LDX_DIR,    &&STX_DIR,    // 0xBX
        &&SUB_EXT,    &&CMP_EXT,    &&SBC_EXT,    &&CPX_EXT,    &&AND_EXT,    &&BIT_EXT,    &&LDA_EXT,    &&STA_EXT,    &&EOR_EXT,    &&ADC_EXT,    &&ORA_EXT,    &&ADD_EXT,    &&JMP_EXT,    &&JSR_EXT,    &&LDX_EXT,    &&STX_EXT,    // 0xCX
        &&SUB_IX2,    &&CMP_IX2,    &&SBC_IX2,    &&CPX_IX2,    &&AND_IX2,    &&BIT_IX2,    &&LDA_IX2,    &&STA_IX2,    &&EOR_IX2,    &&ADC_IX2,    &&ORA_IX2,    &&ADD_IX2,    &&JMP_IX2,    &&JSR_IX2,    &&LDX_IX2,    &&STX_IX2,    // 0xDX
        &&SUB_IX1,    &&CMP_IX1,    &&SBC_IX1,    &&CPX_IX1,    &&AND_IX1,    &&BIT_IX1,    &&LDA_IX1,    &&STA_IX1,    &&EOR_IX1,    &&ADC_IX1,    &&ORA_IX1,    &&ADD_IX1,    &&JMP_IX1,    &&JSR_IX1,    &&LDX_IX1,    &&STX_IX1,    // 0xEX
        &&SUB_IX,     &&CMP_IX,     &&SBC_IX,     &&CPX_IX,     &&AND_IX,     &&BIT_IX,     &&LDA_IX,     &&STA_IX,     &&EOR_IX,     &&ADC_IX,     &&ORA_IX,     &&ADD_IX,     &&JMP_IX,     &&JSR_IX,     &&LDX_IX,     &&STX_IX,     // 0xFX
    };

    if(!CCR[CCRI] && !maskableInterrupts.empty())
    {
        const std::set<uint16_t, std::greater<uint16_t>>::iterator it = maskableInterrupts.begin();
        ProcessInterrupt(*it);
        maskableInterrupts.erase(it);
    }

    const uint8_t opcode = GetNextByte();
    goto *ITC[opcode];

    // 0x0X
    BRSET0_DIR:
        BRSET<0>();
//        LOG(fprintf(instructions, "%X\tBRSET0 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRCLR0_DIR:
        BRCLR<0>();
//        LOG(fprintf(instructions, "%X\tBRCLR0 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRSET1_DIR:
        BRSET<1>();
//        LOG(fprintf(instructions, "%X\tBRSET1 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRCLR1_DIR:
        BRCLR<1>();
//        LOG(fprintf(instructions, "%X\tBRCLR1 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRSET2_DIR:
        BRSET<2>();
//        LOG(fprintf(instructions, "%X\tBRSET2 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRCLR2_DIR:
        BRCLR<2>();
//        LOG(fprintf(instructions, "%X\tBRCLR2 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRSET3_DIR:
        BRSET<3>();
//        LOG(fprintf(instructions, "%X\tBRSET3 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRCLR3_DIR:
        BRCLR<3>();
//        LOG(fprintf(instructions, "%X\tBRCLR3 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRSET4_DIR:
        BRSET<4>();
//        LOG(fprintf(instructions, "%X\tBRSET4 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRCLR4_DIR:
        BRCLR<4>();
//        LOG(fprintf(instructions, "%X\tBRCLR4 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRSET5_DIR:
        BRSET<5>();
//        LOG(fprintf(instructions, "%X\tBRSET5 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRCLR5_DIR:
        BRCLR<5>();
//        LOG(fprintf(instructions, "%X\tBRCLR5 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRSET6_DIR:
        BRSET<6>();
//        LOG(fprintf(instructions, "%X\tBRSET6 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRCLR6_DIR:
        BRCLR<6>();
//        LOG(fprintf(instructions, "%X\tBRCLR6 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRSET7_DIR:
        BRSET<7>();
//        LOG(fprintf(instructions, "%X\tBRSET7 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    BRCLR7_DIR:
        BRCLR<7>();
//        LOG(fprintf(instructions, "%X\tBRCLR7 0x%X, %d\n", currentPC, addr, offset);)
        return 5;

    // 0x1X
    BSET0_DIR:
        BSET<0>();
//        LOG(fprintf(instructions, "%X\tBSET0 0x%X\n", currentPC, addr);)
        return 5;

    BCLR0_DIR:
        BCLR<0>();
//        LOG(fprintf(instructions, "%X\tBCLR0 0x%X\n", currentPC, addr);)
        return 5;

    BSET1_DIR:
        BSET<1>();
//        LOG(fprintf(instructions, "%X\tBSET1 0x%X\n", currentPC, addr);)
        return 5;

    BCLR1_DIR:
        BCLR<1>();
//        LOG(fprintf(instructions, "%X\tBCLR1 0x%X\n", currentPC, addr);)
        return 5;

    BSET2_DIR:
        BSET<2>();
//        LOG(fprintf(instructions, "%X\tBSET2 0x%X\n", currentPC, addr);)
        return 5;

    BCLR2_DIR:
        BCLR<2>();
//        LOG(fprintf(instructions, "%X\tBCLR2 0x%X\n", currentPC, addr);)
        return 5;

    BSET3_DIR:
        BSET<3>();
//        LOG(fprintf(instructions, "%X\tBSET3 0x%X\n", currentPC, addr);)
        return 5;

    BCLR3_DIR:
        BCLR<3>();
//        LOG(fprintf(instructions, "%X\tBCLR3 0x%X\n", currentPC, addr);)
        return 5;

    BSET4_DIR:
        BSET<4>();
//        LOG(fprintf(instructions, "%X\tBSET4 0x%X\n", currentPC, addr);)
        return 5;

    BCLR4_DIR:
        BCLR<4>();
//        LOG(fprintf(instructions, "%X\tBCLR4 0x%X\n", currentPC, addr);)
        return 5;

    BSET5_DIR:
        BSET<5>();
//        LOG(fprintf(instructions, "%X\tBSET5 0x%X\n", currentPC, addr);)
        return 5;

    BCLR5_DIR:
        BCLR<5>();
//        LOG(fprintf(instructions, "%X\tBCLR5 0x%X\n", currentPC, addr);)
        return 5;

    BSET6_DIR:
        BSET<6>();
//        LOG(fprintf(instructions, "%X\tBSET6 0x%X\n", currentPC, addr);)
        return 5;

    BCLR6_DIR:
        BCLR<6>();
//        LOG(fprintf(instructions, "%X\tBCLR6 0x%X\n", currentPC, addr);)
        return 5;

    BSET7_DIR:
        BSET<7>();
//        LOG(fprintf(instructions, "%X\tBSET7 0x%X\n", currentPC, addr);)
        return 5;

    BCLR7_DIR:
        BCLR<7>();
//        LOG(fprintf(instructions, "%X\tBCLR7 0x%X\n", currentPC, addr);)
        return 5;

    // 0x2X
    BRA_REL:
        Branch(true);
//        LOG(fprintf(instructions, "%X\tBRA %d\n", currentPC, offset);)
        return 3;

    BRN_REL:
        Branch(false);
//        LOG(fprintf(instructions, "%X\tBRN\n", currentPC);)
        return 3;

    BHI_REL:
        Branch(!CCR[CCRC] && !CCR[CCRZ]);
//        LOG(fprintf(instructions, "%X\tBHI %d\n", currentPC, offset);)
        return 3;

    BLS_REL:
        Branch(CCR[CCRC] || CCR[CCRZ]);
//        LOG(fprintf(instructions, "%X\tBLS %d\n", currentPC, offset);)
        return 3;

    BCC_REL:
        Branch(!CCR[CCRC]);
//        LOG(fprintf(instructions, "%X\tBCC %d\n", currentPC, offset);)
        return 3;

    BCS_REL:
        Branch(CCR[CCRC]);
//        LOG(fprintf(instructions, "%X\tBLO/BCS %d\n", currentPC, offset);)
        return 3;

    BNE_REL:
        Branch(!CCR[CCRZ]);
//        LOG(fprintf(instructions, "%X\tBNE %d\n", currentPC, offset);)
        return 3;

    BEQ_REL:
        Branch(CCR[CCRZ]);
//        LOG(fprintf(instructions, "%X\tBEQ %d\n", currentPC, offset);)
        return 3;

    BHCC_REL:
        Branch(!CCR[CCRH]);
//        LOG(fprintf(instructions, "%X\tBHCC %d\n", currentPC, offset);)
        return 3;

    BHCS_REL:
        Branch(CCR[CCRH]);
//        LOG(fprintf(instructions, "%X\tBHCS %d\n", currentPC, offset);)
        return 3;

    BPL_REL:
        Branch(!CCR[CCRN]);
//        LOG(fprintf(instructions, "%X\tBPL %d\n", currentPC, offset);)
        return 3;

    BMI_REL:
        Branch(CCR[CCRN]);
//        LOG(fprintf(instructions, "%X\tBMI %d\n", currentPC, offset);)
        return 3;

    BMC_REL:
        Branch(!CCR[CCRI]);
//        LOG(fprintf(instructions, "%X\tBMC %d\n", currentPC, offset);)
        return 3;

    BMS_REL:
        Branch(CCR[CCRI]);
//        LOG(fprintf(instructions, "%X\tBMS %d\n", currentPC, offset);)
        return 3;

    BIL_REL:
        Branch(!irqPin);
//        LOG(fprintf(instructions, "%X\tBIL %d\n", currentPC, offset);)
        return 3;

    BIH_REL:
        Branch(irqPin);
//        LOG(fprintf(instructions, "%X\tBIH %d\n", currentPC, offset);)
        return 3;

    // 0x3X
    NEG_DIR:
        NEG(EA_DIR());
//        LOG(fprintf(instructions, "%X\tNEG 0x%X\n", currentPC, addr);)
        return 5;

    COM_DIR:
        COM(EA_DIR());
//        LOG(fprintf(instructions, "%X\tCOM 0x%X\n", currentPC, addr);)
        return 5;

    LSR_DIR:
        LSR(EA_DIR());
//        LOG(fprintf(instructions, "%X\tLSR 0x%X\n", currentPC, addr);)
        return 5;

    ROR_DIR:
        ROR(EA_DIR());
//        LOG(fprintf(instructions, "%X\tROR 0x%X\n", currentPC, addr);)
        return 5;

    ASR_DIR:
        ASR(EA_DIR());
//        LOG(fprintf(instructions, "%X\tASR 0x%X\n", currentPC, addr);)
        return 5;

    LSL_DIR:
        LSL(EA_DIR());
//        LOG(fprintf(instructions, "%X\tASL/LSL 0x%X\n", currentPC, addr);)
        return 5;

    ROL_DIR:
        ROL(EA_DIR());
//        LOG(fprintf(instructions, "%X\tROL 0x%X\n", currentPC, addr);)
        return 5;

    DEC_DIR:
        DEC(EA_DIR());
//        LOG(fprintf(instructions, "%X\tDEC 0x%X\n", currentPC, addr);)
        return 5;

    INC_DIR:
        INC(EA_DIR());
//        LOG(fprintf(instructions, "%X\tINC 0x%X\n", currentPC, addr);)
        return 5;

    TST_DIR:
        TST(DIR());
//        LOG(fprintf(instructions, "%X\tTST 0x%X\n", currentPC, addr);)
        return 4;

    CLR_DIR:
        CLR(EA_DIR());
//        LOG(fprintf(instructions, "%X\tCLR 0x%X\n", currentPC, addr);)
        return 5;

    // 0x4X
    NEGA_INH:
        NEG(A);
//        LOG(fprintf(instructions, "%X\tNEGA\n", currentPC);)
        return 3;

    MUL_INH:
    {
        const uint16_t result = (uint16_t)X * (uint16_t)A;
        X = result >> 8;
        A = result;
        CCR[CCRH] = false;
        CCR[CCRC] = false;
//        LOG(fprintf(instructions, "%X\tMUL\n", currentPC);)
        return 11;
    }

    COMA_INH:
        COM(A);
//        LOG(fprintf(instructions, "%X\tCOMA\n", currentPC);)
        return 3;

    LSRA_INH:
        LSR(A);
//        LOG(fprintf(instructions, "%X\tLSRA\n", currentPC);)
        return 3;

    RORA_INH:
        ROR(A);
//        LOG(fprintf(instructions, "%X\tRORA\n", currentPC);)
        return 3;

    ASRA_INH:
        ASR(A);
//        LOG(fprintf(instructions, "%X\tASRA\n", currentPC);)
        return 3;

    LSLA_INH:
        LSL(A);
//        LOG(fprintf(instructions, "%X\tASLA/LSLA\n", currentPC);)
        return 3;

    ROLA_INH:
        ROL(A);
//        LOG(fprintf(instructions, "%X\tROLA\n", currentPC);)
        return 3;

    DECA_INH:
        DEC(A);
//        LOG(fprintf(instructions, "%X\tDECA\n", currentPC);)
        return 3;

    INCA_INH:
        INC(A);
//        LOG(fprintf(instructions, "%X\tINCA\n", currentPC);)
        return 3;

    TSTA_INH:
        TST(A);
//        LOG(fprintf(instructions, "%X\tTSTA\n", currentPC);)
        return 3;

    CLRA_INH:
        CLR(A);
//        LOG(fprintf(instructions, "%X\tCLRA\n", currentPC);)
        return 3;

    // 0x5X
    NEGX_INH:
        NEG(X);
//        LOG(fprintf(instructions, "%X\tNEGX\n", currentPC);)
        return 3;

    COMX_INH:
        COM(X);
//        LOG(fprintf(instructions, "%X\tCOMX\n", currentPC);)
        return 3;

    LSRX_INH:
        LSR(X);
//        LOG(fprintf(instructions, "%X\tLSRX\n", currentPC);)
        return 3;

    RORX_INH:
        ROR(X);
//        LOG(fprintf(instructions, "%X\tRORX\n", currentPC);)
        return 3;

    ASRX_INH:
        ASR(X);
//        LOG(fprintf(instructions, "%X\tASRX\n", currentPC);)
        return 3;

    LSLX_INH:
        LSL(X);
//        LOG(fprintf(instructions, "%X\tASLX/LSLX\n", currentPC);)
        return 3;

    ROLX_INH:
        ROL(X);
//        LOG(fprintf(instructions, "%X\tROLX\n", currentPC);)
        return 3;

    DECX_INH:
        DEC(X);
//        LOG(fprintf(instructions, "%X\tDECX\n", currentPC);)
        return 3;

    INCX_INH:
        INC(X);
//        LOG(fprintf(instructions, "%X\tINCX\n", currentPC);)
        return 3;

    TSTX_INH:
        TST(X);
//        LOG(fprintf(instructions, "%X\tTSTX\n", currentPC);)
        return 3;

    CLRX_INH:
        CLR(X);
//        LOG(fprintf(instructions, "%X\tCLRX\n", currentPC);)
        return 3;

    // 0x6X
    NEG_IX1:
        NEG(EA_IX1());
//        LOG(fprintf(instructions, "%X\tNEG 0x%X, X\n", currentPC, offset);)
        return 6;

    COM_IX1:
        COM(EA_IX1());
//        LOG(fprintf(instructions, "%X\tCOM 0x%X, X\n", currentPC, offset);)
        return 6;

    LSR_IX1:
        LSR(EA_IX1());
//        LOG(fprintf(instructions, "%X\tLSR 0x%X, X\n", currentPC, offset);)
        return 6;

    ROR_IX1:
        ROR(EA_IX1());
//        LOG(fprintf(instructions, "%X\tROR 0x%X, X\n", currentPC, offset);)
        return 6;

    ASR_IX1:
        ASR(EA_IX1());
//        LOG(fprintf(instructions, "%X\tASR 0x%X, X\n", currentPC, offset);)
        return 6;

    LSL_IX1:
        LSL(EA_IX1());
//        LOG(fprintf(instructions, "%X\tASL/LSL 0x%X, X\n", currentPC, offset);)
        return 6;

    ROL_IX1:
        ROL(EA_IX1());
//        LOG(fprintf(instructions, "%X\tROL 0x%X, X\n", currentPC, offset);)
        return 6;

    DEC_IX1:
        DEC(EA_IX1());
//        LOG(fprintf(instructions, "%X\tDEC 0x%X, X\n", currentPC, offset);)
        return 6;

    INC_IX1:
        INC(EA_IX1());
//        LOG(fprintf(instructions, "%X\tINC 0x%X, X\n", currentPC, offset);)
        return 6;

    TST_IX1:
        TST(IX1());
//        LOG(fprintf(instructions, "%X\tTST 0x%X, X\n", currentPC, offset);)
        return 5;

    CLR_IX1:
        CLR(EA_IX1());
//        LOG(fprintf(instructions, "%X\tCLR 0x%X, X\n", currentPC, offset);)
        return 6;

    // 0x7X
    NEG_IX:
        NEG(EA_IX());
//        LOG(fprintf(instructions, "%X\tNEG X\n", currentPC);)
        return 5;

    COM_IX:
        COM(EA_IX());
//        LOG(fprintf(instructions, "%X\tCOM X\n", currentPC);)
        return 5;

    LSR_IX:
        LSR(EA_IX());
//        LOG(fprintf(instructions, "%X\tLSR X\n", currentPC);)
        return 5;

    ROR_IX:
        ROR(EA_IX());
//        LOG(fprintf(instructions, "%X\tROR X\n", currentPC);)
        return 5;

    ASR_IX:
        ASR(EA_IX());
//        LOG(fprintf(instructions, "%X\tASR X\n", currentPC);)
        return 5;

    LSL_IX:
        LSL(EA_IX());
//        LOG(fprintf(instructions, "%X\tASL/LSL X\n", currentPC);)
        return 5;

    ROL_IX:
        ROL(EA_IX());
//        LOG(fprintf(instructions, "%X\tROL X\n", currentPC);)
        return 5;

    DEC_IX:
        DEC(EA_IX());
//        LOG(fprintf(instructions, "%X\tDEC X\n", currentPC);)
        return 5;

    INC_IX:
        INC(EA_IX());
//        LOG(fprintf(instructions, "%X\tINC X\n", currentPC);)
        return 5;

    TST_IX:
        TST(IX());
//        LOG(fprintf(instructions, "%X\tTST X\n", currentPC);)
        return 4;

    CLR_IX:
        CLR(EA_IX());
//        LOG(fprintf(instructions, "%X\tCLR X\n", currentPC);)
        return 5;

    // 0x8X
    RTI_INH:
    {
        CCR = PopByte() | 0xE0;
        A = PopByte();
        X = PopByte();
        PC = (uint16_t)PopByte() << 8;
        PC |= PopByte();
//        LOG(fprintf(instructions, "%X\tRTI\n", currentPC);)
        return 9;
    }

    RTS_INH:
    {
        PC = (uint16_t)PopByte() << 8;
        PC |= PopByte();
//        LOG(fprintf(instructions, "%X\tRTS\n", currentPC);)
        return 6;
    }

    SWI_INH:
    {
        ProcessInterrupt(memorySize - 4); // SWI is non-maskable.
//        LOG(fprintf(instructions, "%X\tSWI\n", currentPC);)
        return 10;
    }

    STOP_INH:
    {
        CCR[CCRI] = false;
        stop = true;
        Stop();
//        LOG(fprintf(instructions, "%X\tSTOP\n", currentPC);)
        return 2;
    }

    WAIT_INH:
    {
        CCR[CCRI] = false;
        wait = true;
        Wait();
//        LOG(fprintf(instructions, "%X\tWAIT\n", currentPC);)
        return 2;
    }

    // 0x9X
    TAX_INH:
        X = A;
//        LOG(fprintf(instructions, "%X\tTAX\n", currentPC);)
        return 2;

    CLC_INH:
        CCR[CCRC] = false;
//        LOG(fprintf(instructions, "%X\tCLC\n", currentPC);)
        return 2;

    SEC_INH:
        CCR[CCRC] = true;
//        LOG(fprintf(instructions, "%X\tSEC\n", currentPC);)
        return 2;

    CLI_INH:
        CCR[CCRI] = false;
//        LOG(fprintf(instructions, "%X\tCLI\n", currentPC);)
        return 2;

    SEI_INH:
        CCR[CCRI] = true;
//        LOG(fprintf(instructions, "%X\tSEI\n", currentPC);)
        return 2;

    RSP_INH:
        SP = 0xFF;
//        LOG(fprintf(instructions, "%X\tRSP\n", currentPC);)
        return 2;

    NOP_INH:
//        LOG(fprintf(instructions, "%X\tNOP\n", currentPC);)
        return 2;

    TXA_INH:
        A = X;
//        LOG(fprintf(instructions, "%X\tTAX\n", currentPC);)
        return 2;

    // 0xAX
    SUB_IMM:
        SUB(IMM());
//        LOG(fprintf(instructions, "%X\tSUB #0x%X\n", currentPC, data);)
        return 2;

    CMP_IMM:
        CMP(A, IMM());
//        LOG(fprintf(instructions, "%X\tCMP #0x%X\n", currentPC, data);)
        return 2;

    SBC_IMM:
        SBC(IMM());
//        LOG(fprintf(instructions, "%X\tSBC #0x%X\n", currentPC, data);)
        return 2;

    CPX_IMM:
        CMP(X, IMM());
//        LOG(fprintf(instructions, "%X\tCPX #0x%X\n", currentPC, data);)
        return 2;

    AND_IMM:
        AND(IMM());
//        LOG(fprintf(instructions, "%X\tAND #0x%X\n", currentPC, data);)
        return 2;

    BIT_IMM:
        BIT(IMM());
        return 2;

    LDA_IMM:
        LDAX(A, IMM());
//        LOG(fprintf(instructions, "%X\tLDA #0x%X\n", currentPC, A);)
        return 2;

    EOR_IMM:
        EOR(IMM());
//        LOG(fprintf(instructions, "%X\tEOR #0x%X\n", currentPC, data);)
        return 2;

    ADC_IMM:
        ADC(IMM());
//        LOG(fprintf(instructions, "%X\tADC #0x%X\n", currentPC, data);)
        return 2;

    ORA_IMM:
        ORA(IMM());
//        LOG(fprintf(instructions, "%X\tORA #0x%X\n", currentPC, data);)
        return 2;

    ADD_IMM:
        ADD(IMM());
//        LOG(fprintf(instructions, "%X\tADD #0x%X\n", currentPC, data);)
        return 2;

    BSR_REL:
    {
        const int8_t offset = GetNextByte();
        PushByte(PC);
        PushByte(PC >> 8);
        PC += offset;
//        LOG(fprintf(instructions, "%X\tBSR %d\n", currentPC, offset);)
        return 6;
    }

    LDX_IMM:
        LDAX(X, IMM());
//        LOG(fprintf(instructions, "%X\tLDX #0x%X\n", currentPC, X);)
        return 2;

    // 0xBX
    SUB_DIR:
        SUB(DIR());
//        LOG(fprintf(instructions, "%X\tSUB 0x%X\n", currentPC, addr);)
        return 3;

    CMP_DIR:
        CMP(A, DIR());
//        LOG(fprintf(instructions, "%X\tCMP 0x%X\n", currentPC, addr);)
        return 3;

    SBC_DIR:
        SBC(DIR());
//        LOG(fprintf(instructions, "%X\tSBC 0x%X\n", currentPC, addr);)
        return 3;

    CPX_DIR:
        CMP(X, DIR());
//        LOG(fprintf(instructions, "%X\tCPX 0x%X\n", currentPC, addr);)
        return 3;

    AND_DIR:
        AND(DIR());
//        LOG(fprintf(instructions, "%X\tAND 0x%X\n", currentPC, addr);)
        return 3;

    BIT_DIR:
        BIT(DIR());
//        LOG(fprintf(instructions, "%X\tBIT 0x%X\n", currentPC, addr);)
        return 3;

    LDA_DIR:
        LDAX(A, DIR());
//        LOG(fprintf(instructions, "%X\tLDA 0x%X\n", currentPC, addr);)
        return 3;

    STA_DIR:
        STAX(A, EA_DIR());
//        LOG(fprintf(instructions, "%X\tSTA 0x%X\n", currentPC, addr);)
        return 4;

    EOR_DIR:
        EOR(DIR());
//        LOG(fprintf(instructions, "%X\tEOR 0x%X\n", currentPC, addr);)
        return 3;

    ADC_DIR:
        ADC(DIR());
//        LOG(fprintf(instructions, "%X\tADC 0x%X\n", currentPC, addr);)
        return 3;

    ORA_DIR:
        ORA(DIR());
//        LOG(fprintf(instructions, "%X\tORA 0x%X\n", currentPC, addr);)
        return 3;

    ADD_DIR:
        ADD(DIR());
//        LOG(fprintf(instructions, "%X\tADD 0x%X\n", currentPC, addr);)
        return 3;

    JMP_DIR:
        JMP(EA_DIR());
//        LOG(fprintf(instructions, "%X\tJMP 0x%X\n", currentPC, addr);)
        return 2;

    JSR_DIR:
        JSR(EA_DIR());
//        LOG(fprintf(instructions, "%X\tJSR 0x%X\n", currentPC, addr);)
        return 5;

    LDX_DIR:
        LDAX(X, DIR());
//        LOG(fprintf(instructions, "%X\tLDX 0x%X\n", currentPC, addr);)
        return 3;

    STX_DIR:
        STAX(X, EA_DIR());
//        LOG(fprintf(instructions, "%X\tSTX 0x%X\n", currentPC, addr);)
        return 4;

    // 0xCX
    SUB_EXT:
        SUB(EXT());
//        LOG(fprintf(instructions, "%X\tSUB 0x%X\n", currentPC, addr);)
        return 4;

    CMP_EXT:
        CMP(A, EXT());
//        LOG(fprintf(instructions, "%X\tCMP 0x%X\n", currentPC, addr);)
        return 4;

    SBC_EXT:
        SBC(EXT());
//        LOG(fprintf(instructions, "%X\tSBC 0x%X\n", currentPC, addr);)
        return 4;

    CPX_EXT:
        CMP(X, EXT());
//        LOG(fprintf(instructions, "%X\tCPX 0x%X\n", currentPC, addr);)
        return 4;

    AND_EXT:
        AND(EXT());
//        LOG(fprintf(instructions, "%X\tAND 0x%X\n", currentPC, addr);)
        return 4;

    BIT_EXT:
        BIT(EXT());
//        LOG(fprintf(instructions, "%X\tBIT 0x%X\n", currentPC, addr);)
        return 4;

    LDA_EXT:
        LDAX(A, EXT());
//        LOG(fprintf(instructions, "%X\tLDA 0x%X\n", currentPC, addr);)
        return 4;

    STA_EXT:
        STAX(A, EA_EXT());
//        LOG(fprintf(instructions, "%X\tSTA 0x%X\n", currentPC, addr);)
        return 5;

    EOR_EXT:
        EOR(EXT());
//        LOG(fprintf(instructions, "%X\tEOR 0x%X\n", currentPC, addr);)
        return 4;

    ADC_EXT:
        ADC(EXT());
//        LOG(fprintf(instructions, "%X\tADC 0x%X\n", currentPC, addr);)
        return 4;

    ORA_EXT:
        ORA(EXT());
//        LOG(fprintf(instructions, "%X\tORA 0x%X\n", currentPC, addr);)
        return 4;

    ADD_EXT:
        ADD(EXT());
//        LOG(fprintf(instructions, "%X\tADD 0x%X\n", currentPC, addr);)
        return 4;

    JMP_EXT:
        JMP(EA_EXT());
//        LOG(fprintf(instructions, "%X\tJMP 0x%X\n", currentPC, addr);)
        return 3;

    JSR_EXT:
        JSR(EA_EXT());
//        LOG(fprintf(instructions, "%X\tJSR 0x%X\n", currentPC, addr);)
        return 6;

    LDX_EXT:
        LDAX(X, EXT());
//        LOG(fprintf(instructions, "%X\tLDX 0x%X\n", currentPC, addr);)
        return 4;

    STX_EXT:
        STAX(X, EA_EXT());
//        LOG(fprintf(instructions, "%X\tSTX 0x%X\n", currentPC, addr);)
        return 5;

    // 0xDX
    SUB_IX2:
        SUB(IX2());
//        LOG(fprintf(instructions, "%X\tSUB 0x%X, X\n", currentPC, offset);)
        return 5;

    CMP_IX2:
        CMP(A, IX2());
//        LOG(fprintf(instructions, "%X\tCMP 0x%X, X\n", currentPC, offset);)
        return 5;

    SBC_IX2:
        SBC(IX2());
//        LOG(fprintf(instructions, "%X\tSBC 0x%X, X\n", currentPC, offset);)
        return 5;

    CPX_IX2:
        CMP(X, IX2());
//        LOG(fprintf(instructions, "%X\tCPX 0x%X, X\n", currentPC, offset);)
        return 5;

    AND_IX2:
        AND(IX2());
//        LOG(fprintf(instructions, "%X\tAND 0x%X, X\n", currentPC, offset);)
        return 5;

    BIT_IX2:
        BIT(IX2());
//        LOG(fprintf(instructions, "%X\tBIT 0x%X, X\n", currentPC, offset);)
        return 5;

    LDA_IX2:
        LDAX(A, IX2());
//        LOG(fprintf(instructions, "%X\tLDA 0x%X, X\n", currentPC, offset);)
        return 5;

    STA_IX2:
        STAX(A, EA_IX2());
//        LOG(fprintf(instructions, "%X\tSTA 0x%X, X\n", currentPC, offset);)
        return 6;

    EOR_IX2:
        EOR(IX2());
//        LOG(fprintf(instructions, "%X\tEOR 0x%X, X\n", currentPC, offset);)
        return 5;

    ADC_IX2:
        ADC(IX2());
//        LOG(fprintf(instructions, "%X\tADC 0x%X, X\n", currentPC, offset);)
        return 5;

    ORA_IX2:
        ORA(IX2());
//        LOG(fprintf(instructions, "%X\tORA 0x%X, X\n", currentPC, offset);)
        return 5;

    ADD_IX2:
        ADD(IX2());
//        LOG(fprintf(instructions, "%X\tADD 0x%X, X\n", currentPC, offset);)
        return 5;

    JMP_IX2:
        JMP(EA_IX2());
//        LOG(fprintf(instructions, "%X\tJMP 0x%X, X\n", currentPC, offset);)
        return 4;

    JSR_IX2:
        JSR(EA_IX2());
//        LOG(fprintf(instructions, "%X\tJSR 0x%X, X\n", currentPC, offset);)
        return 7;

    LDX_IX2:
        LDAX(X, IX2());
//        LOG(fprintf(instructions, "%X\tLDX 0x%X, X\n", currentPC, offset);)
        return 5;

    STX_IX2:
        STAX(X, EA_IX2());
//        LOG(fprintf(instructions, "%X\tSTX 0x%X, X\n", currentPC, offset);)
        return 6;

    // 0xEX
    SUB_IX1:
        SUB(IX1());
//        LOG(fprintf(instructions, "%X\tSUB 0x%X, X\n", currentPC, offset);)
        return 4;

    CMP_IX1:
        CMP(A, IX1());
//        LOG(fprintf(instructions, "%X\tCMP 0x%X, X\n", currentPC, offset);)
        return 4;

    SBC_IX1:
        SBC(IX1());
//        LOG(fprintf(instructions, "%X\tSBC 0x%X, X\n", currentPC, offset);)
        return 4;

    CPX_IX1:
        CMP(X, IX1());
//        LOG(fprintf(instructions, "%X\tCPX 0x%X, X\n", currentPC, offset);)
        return 4;

    AND_IX1:
        AND(IX1());
//        LOG(fprintf(instructions, "%X\tAND 0x%X, X\n", currentPC, offset);)
        return 4;

    BIT_IX1:
        BIT(IX1());
//        LOG(fprintf(instructions, "%X\tBIT 0x%X, X\n", currentPC, offset);)
        return 4;

    LDA_IX1:
        LDAX(A, IX1());
//        LOG(fprintf(instructions, "%X\tLDA 0x%X, X\n", currentPC, offset);)
        return 4;

    STA_IX1:
        STAX(A, EA_IX1());
//        LOG(fprintf(instructions, "%X\tSTA 0x%X, X\n", currentPC, offset);)
        return 5;

    EOR_IX1:
        EOR(IX1());
//        LOG(fprintf(instructions, "%X\tEOR 0x%X, X\n", currentPC, offset);)
        return 4;

    ADC_IX1:
        ADC(IX1());
//        LOG(fprintf(instructions, "%X\tADC 0x%X, X\n", currentPC, offset);)
        return 4;

    ORA_IX1:
        ORA(IX1());
//        LOG(fprintf(instructions, "%X\tORA 0x%X, X\n", currentPC, offset);)
        return 4;

    ADD_IX1:
        ADD(IX1());
//        LOG(fprintf(instructions, "%X\tADD 0x%X, X\n", currentPC, offset);)
        return 4;

    JMP_IX1:
        JMP(EA_IX1());
//        LOG(fprintf(instructions, "%X\tJMP 0x%X, X\n", currentPC, offset);)
        return 3;

    JSR_IX1:
        JSR(EA_IX1());
//        LOG(fprintf(instructions, "%X\tJSR 0x%X, X\n", currentPC, offset);)
        return 6;

    LDX_IX1:
        LDAX(X, IX1());
//        LOG(fprintf(instructions, "%X\tLDX 0x%X, X\n", currentPC, offset);)
        return 4;

    STX_IX1:
        STAX(X, EA_IX1());
//        LOG(fprintf(instructions, "%X\tSTX 0x%X, X\n", currentPC, offset);)
        return 5;

    // 0xFX
    SUB_IX:
        SUB(IX());
//        LOG(fprintf(instructions, "%X\tSUB X\n", currentPC);)
        return 3;

    CMP_IX:
        CMP(A, IX());
//        LOG(fprintf(instructions, "%X\tCMP X\n", currentPC);)
        return 3;

    SBC_IX:
        SBC(IX());
//        LOG(fprintf(instructions, "%X\tSBC X\n", currentPC);)
        return 3;

    CPX_IX:
        CMP(X, IX());
//        LOG(fprintf(instructions, "%X\tCPX X\n", currentPC);)
        return 3;

    AND_IX:
        AND(IX());
//        LOG(fprintf(instructions, "%X\tAND X\n", currentPC);)
        return 3;

    BIT_IX:
        BIT(IX());
//        LOG(fprintf(instructions, "%X\tBIT X\n", currentPC);)
        return 3;

    LDA_IX:
        LDAX(A, IX());
//        LOG(fprintf(instructions, "%X\tLDA X\n", currentPC);)
        return 3;

    STA_IX:
        STAX(A, EA_IX());
//        LOG(fprintf(instructions, "%X\tSTA X\n", currentPC);)
        return 4;

    EOR_IX:
        EOR(IX());
//        LOG(fprintf(instructions, "%X\tEOR X\n", currentPC);)
        return 3;

    ADC_IX:
        ADC(IX());
//        LOG(fprintf(instructions, "%X\tADC X\n", currentPC);)
        return 3;

    ORA_IX:
        ORA(IX());
//        LOG(fprintf(instructions, "%X\tORA X\n", currentPC);)
        return 3;

    ADD_IX:
        ADD(IX());
//        LOG(fprintf(instructions, "%X\tADD X\n", currentPC);)
        return 3;

    JMP_IX:
        JMP(EA_IX());
//        LOG(fprintf(instructions, "%X\tJMP X\n", currentPC);)
        return 2;

    JSR_IX:
        JSR(EA_IX());
//        LOG(fprintf(instructions, "%X\tJSR X\n", currentPC);)
        return 5;

    LDX_IX:
        LDAX(X, IX());
//        LOG(fprintf(instructions, "%X\tLDX X\n", currentPC);)
        return 3;

    STX_IX:
        STAX(X, EA_IX());
//        LOG(fprintf(instructions, "%X\tSTX X\n", currentPC);)
        return 4;

    unknown:
        printf("[MC68HC05] %X\tUnknwon instruction: 0x%X\n", PC - 1, opcode);
    return 0;
}

void MC68HC05::RequestInterrupt(uint16_t addr)
{
    maskableInterrupts.insert(addr);
}

/** \brief Processes an interrupt.
 *  \param addr The address where to fetch the high byte of the vector.
 */
void MC68HC05::ProcessInterrupt(uint16_t addr)
{
    PushByte(PC);
    PushByte(PC >> 8);
    PushByte(X);
    PushByte(A);
    PushByte(CCR.to_ulong());
    CCR[CCRI] = true;
    PC = (uint16_t)GetMemory(addr) << 8;
    PC |= GetMemory(addr + 1);
}

void MC68HC05::PushByte(uint8_t data)
{
    SetMemory(SP--, data);
    if(SP < 0xC0)
        SP = 0xFF;
}

uint8_t MC68HC05::PopByte()
{
    if(SP >= 0xFF)
        SP = 0xBF;
    return GetMemory(++SP);
}

uint8_t MC68HC05::GetNextByte()
{
    return GetMemory(PC++);
}

uint16_t MC68HC05::EA_DIR()
{
    return GetNextByte();
}

uint16_t MC68HC05::EA_EXT()
{
    const uint16_t ea = (uint16_t)GetNextByte() << 8;
    return ea | (uint16_t)GetNextByte();
}

uint16_t MC68HC05::EA_IX()
{
    return X;
}

uint16_t MC68HC05::EA_IX1()
{
    return (uint16_t)X + EA_DIR();
}

uint16_t MC68HC05::EA_IX2()
{
    return (uint16_t)X + EA_EXT();
}

uint8_t MC68HC05::IMM()
{
    return GetNextByte();
}

uint8_t MC68HC05::DIR()
{
    return GetMemory(EA_DIR());
}

uint8_t MC68HC05::EXT()
{
    return GetMemory(EA_EXT());
}

uint8_t MC68HC05::IX()
{
    return GetMemory(EA_IX());
}

uint8_t MC68HC05::IX1()
{
    return GetMemory(EA_IX1());
}

uint8_t MC68HC05::IX2()
{
    return GetMemory(EA_IX2());
}

#define CARRY_HALF_ADD(a, m, r, mask) (((a & mask) && (m & mask)) || ((m & mask) && !(r & mask)) || (!(r & mask) && (a & mask)))
#define CARRY_SUB(a, m, r)            ((!(a & 0x80) && (m & 0x80)) || ((m & 0x80) && (r & 0x80)) || ((r & 0x80) && !(a & 0x80)))

void MC68HC05::ADC(uint8_t rhs)
{
    ADD(rhs + CCR[CCRC]);
}

void MC68HC05::ADD(uint8_t rhs)
{
    const uint8_t res = A + rhs;
    CCR[CCRH] = CARRY_HALF_ADD(A, rhs, res, 0x08);
    CCR[CCRN] = res & 0x80;
    CCR[CCRZ] = res == 0;
    CCR[CCRC] = CARRY_HALF_ADD(A, rhs, res, 0x80);
    A = res;
}

void MC68HC05::AND(uint8_t rhs)
{
    A = BIT(rhs);
}

void MC68HC05::ASR(uint8_t& reg)
{
    CCR[CCRC] = reg & 1;
    const uint8_t msb = reg & 0x80;
    reg >>= 1;
    reg |= msb;
    CCR[CCRN] = reg & 0x80;
    CCR[CCRZ] = reg == 0;
}

void MC68HC05::ASR(uint16_t addr)
{
    uint8_t data = GetMemory(addr);
    CCR[CCRC] = data & 1;
    const uint8_t msb = data & 0x80;
    data >>= 1;
    data |= msb;
    CCR[CCRN] = data & 0x80;
    CCR[CCRZ] = data == 0;
    SetMemory(addr, data);
}

template<int BITNUM>
void MC68HC05::BCLR()
{
    const uint8_t addr = GetNextByte();
    SetMemory(addr, GetMemory(addr) & ~(1 << BITNUM));
}

uint8_t MC68HC05::BIT(uint8_t rhs)
{
    const uint8_t res = A & rhs;
    CCR[CCRN] = res & 0x80;
    CCR[CCRZ] = res == 0;
    return res;
}

void MC68HC05::Branch(bool condition)
{
    const int8_t offset = GetNextByte();
    if(condition)
        PC += offset;
}

template<int BITNUM>
void MC68HC05::BRCLR()
{
    const uint8_t addr = GetNextByte();
    const int8_t offset = GetNextByte();
    CCR[CCRC] = GetMemory(addr) & (1 << BITNUM);
    if(!CCR[CCRC])
        PC += offset;
}

template<int BITNUM>
void MC68HC05::BRSET()
{
    const uint8_t addr = GetNextByte();
    const int8_t offset = GetNextByte();
    CCR[CCRC] = GetMemory(addr) & (1 << BITNUM);
    if(CCR[CCRC])
        PC += offset;
}

template<int BITNUM>
void MC68HC05::BSET()
{
    const uint8_t addr = GetNextByte();
    SetMemory(addr, GetMemory(addr) | (1 << BITNUM));
}

void MC68HC05::CLR(uint8_t& reg)
{
    reg = 0;
    CCR[CCRN] = false;
    CCR[CCRZ] = true;
}

void MC68HC05::CLR(uint16_t addr)
{
    CCR[CCRN] = false;
    CCR[CCRZ] = true;
    SetMemory(addr, 0);
}

uint8_t MC68HC05::CMP(uint8_t lhs, uint8_t rhs)
{
    const uint8_t res = lhs - rhs;
    CCR[CCRN] = res & 0x80;
    CCR[CCRZ] = res == 0;
    CCR[CCRC] = CARRY_SUB(lhs, rhs, res);
    return res;
}

void MC68HC05::COM(uint8_t& reg)
{
    reg = !reg;
    CCR[CCRN] = reg & 0x80;
    CCR[CCRZ] = reg == 0;
    CCR[CCRC] = true;
}

void MC68HC05::COM(uint16_t addr)
{
    const uint8_t res = !GetMemory(addr);
    CCR[CCRN] = res & 0x80;
    CCR[CCRZ] = res == 0;
    CCR[CCRC] = true;
    SetMemory(addr, res);
}

void MC68HC05::DEC(uint8_t& reg)
{
    reg--;
    CCR[CCRN] = reg & 0x80;
    CCR[CCRZ] = reg == 0;
}

void MC68HC05::DEC(uint16_t addr)
{
    const uint8_t res = GetMemory(addr) - 1;
    CCR[CCRN] = res & 0x80;
    CCR[CCRZ] = res == 0;
    SetMemory(addr, res);
}

void MC68HC05::EOR(uint8_t rhs)
{
    const uint8_t res = A ^ rhs;
    CCR[CCRN] = res & 0x80;
    CCR[CCRZ] = res == 0;
    A = res;
}

void MC68HC05::INC(uint8_t& reg)
{
    reg++;
    CCR[CCRN] = reg & 0x80;
    CCR[CCRZ] = reg == 0;
}

void MC68HC05::INC(uint16_t addr)
{
    const uint8_t res = GetMemory(addr) + 1;
    CCR[CCRN] = res & 0x80;
    CCR[CCRZ] = res == 0;
    SetMemory(addr, res);
}

void MC68HC05::JMP(uint16_t addr)
{
    PC = addr;
}

void MC68HC05::JSR(uint16_t addr)
{
    PushByte(PC);
    PushByte(PC >> 8);
    JMP(addr);
}

void MC68HC05::LDAX(uint8_t& reg, uint8_t rhs)
{
    reg = rhs;
    CCR[CCRN] = rhs & 0x80;
    CCR[CCRZ] = rhs == 0;
}

void MC68HC05::LSL(uint8_t& reg)
{
    CCR[CCRC] = reg & 0x80;
    reg <<= 1;
    CCR[CCRN] = reg & 0x80;
    CCR[CCRZ] = reg == 0;
}

void MC68HC05::LSL(uint16_t addr)
{
    uint8_t data = GetMemory(addr);
    CCR[CCRC] = data & 0x80;
    data <<= 1;
    CCR[CCRN] = data & 0x80;
    CCR[CCRZ] = data == 0;
    SetMemory(addr, data);
}

void MC68HC05::LSR(uint8_t& reg)
{
    CCR[CCRC] = reg & 1;
    reg >>= 1;
    CCR[CCRN] = false;
    CCR[CCRZ] = reg == 0;
}

void MC68HC05::LSR(uint16_t addr)
{
    uint8_t data = GetMemory(addr);
    CCR[CCRC] = data & 1;
    data >>= 1;
    CCR[CCRN] = false;
    CCR[CCRZ] = data == 0;
    SetMemory(addr, data);
}

void MC68HC05::NEG(uint8_t& reg)
{
    reg = -reg;
    CCR[CCRN] = reg & 0x80;
    CCR[CCRZ] = reg == 0;
    CCR[CCRC] = reg != 0;
}

void MC68HC05::NEG(uint16_t addr)
{
    const uint8_t res = -GetMemory(addr);
    CCR[CCRN] = res & 0x80;
    CCR[CCRZ] = res == 0;
    CCR[CCRC] = res != 0;
    SetMemory(addr, res);
}

void MC68HC05::ORA(uint8_t rhs)
{
    const uint8_t res = A | rhs;
    CCR[CCRN] = res & 0x80;
    CCR[CCRZ] = res == 0;
    A = res;
}

void MC68HC05::ROL(uint8_t& reg)
{
    const uint8_t c = CCR[CCRC];
    CCR[CCRC] = reg & 0x80;
    reg <<= 1;
    reg |= c;
    CCR[CCRN] = reg & 0x80;
    CCR[CCRZ] = reg == 0;
}

void MC68HC05::ROL(uint16_t addr)
{
    uint8_t data = GetMemory(addr);
    const uint8_t c = CCR[CCRC];
    CCR[CCRC] = data & 0x80;
    data <<= 1;
    data |= c;
    CCR[CCRN] = data & 0x80;
    CCR[CCRZ] = data == 0;
    SetMemory(addr, data);
}

void MC68HC05::ROR(uint8_t& reg)
{
    const uint8_t c = (uint8_t)CCR[CCRC] << 7;
    CCR[CCRC] = reg & 1;
    reg >>= 1;
    reg |= c;
    CCR[CCRN] = reg & 0x80;
    CCR[CCRZ] = reg == 0;
}

void MC68HC05::ROR(uint16_t addr)
{
    uint8_t data = GetMemory(addr);
    const uint8_t c = (uint8_t)CCR[CCRC] << 7;
    CCR[CCRC] = data & 1;
    data >>= 1;
    data |= c;
    CCR[CCRN] = data & 0x80;
    CCR[CCRZ] = data == 0;
    SetMemory(addr, data);
}

void MC68HC05::SBC(uint8_t rhs)
{
    SUB(rhs + CCR[CCRC]);
}

void MC68HC05::STAX(uint8_t val, uint16_t addr)
{
    SetMemory(addr, val);
    CCR[CCRN] = val & 0x80;
    CCR[CCRZ] = val == 0;
}

void MC68HC05::SUB(uint8_t rhs)
{
    A = CMP(A, rhs);
}

void MC68HC05::TST(uint8_t val)
{
    CCR[CCRN] = val & 0x80;
    CCR[CCRZ] = val == 0;
}
