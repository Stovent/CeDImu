#include "MC68HC705C8.hpp"
#include "../../utils.hpp"

#include <wx/msgdlg.h>

void MC68HC705C8::Execute(const int cycles)
{
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
    BRCLR0_DIR:
    BRSET1_DIR:
    BRCLR1_DIR:
    BRSET2_DIR:
    BRCLR2_DIR:
    BRSET3_DIR:
    BRCLR3_DIR:
    BRSET4_DIR:
    BRCLR4_DIR:
    BRSET5_DIR:
    BRCLR5_DIR:
    BRSET6_DIR:
    BRCLR6_DIR:
    BRSET7_DIR:
    BRCLR7_DIR:

    // 0x1X
    BSET0_DIR:
    BCLR0_DIR:
    BSET1_DIR:
    BCLR1_DIR:
    BSET2_DIR:
    BCLR2_DIR:
    BSET3_DIR:
    BCLR3_DIR:
    BSET4_DIR:
    BCLR4_DIR:
    BSET5_DIR:
    BCLR5_DIR:
    BSET6_DIR:
    BCLR6_DIR:
    BSET7_DIR:
    BCLR7_DIR:

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
        const uint16_t result = A * X;
        X = (result & 0xFF00) >> 8;
        A = result & 0xFF;
        CCR[H] = 0;
        CCR[C] = 0;
        LOG(instructions << std::hex << currentPC << "\tMUL" << std::endl)
        return 1;

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
    RTS_INH:
    SWI_INH:
    STOP_INH:
    WAIT_INH:

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
        SP.byte = 0xFF;
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
    CMP_IMM:
    SBC_IMM:
    CPX_IMM:
    AND_IMM:
    BIT_IMM:
    LDA_IMM:
    EOR_IMM:
    ADC_IMM:
    ORA_IMM:
    ADD_IMM:
    BSR_REL:
    LDX_IMM:

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
