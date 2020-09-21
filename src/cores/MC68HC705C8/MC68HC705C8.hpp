#ifndef MC68HC705C8_HPP
#define MC68HC705C8_HPP

class MC68HC705C8;

#include <cstdint>
#include <string>

class MC68HC705C8
{
public:
    MC68HC705C8();
    ~MC68HC705C8();

    void Reset();
    void Interpreter();

    bool LoadBIOS(const std::string& file);

private:
    uint8_t A;
    uint8_t X;
    uint16_t PC;
    union {
        uint8_t byte;
        struct {
//            const uint8_t unused : 2 = 0b11;
            uint8_t unused : 2;
            uint8_t SP : 6;
        } ;
    } SP;
//    uint8_t SP; // this or upper?
    union {
        uint8_t byte;
        struct {
            : 3;
            uint8_t H : 1;
            uint8_t I : 1;
            uint8_t N : 1;
            uint8_t Z : 1;
            uint8_t C : 1;
        };
    } CCR;
//    uint8_t CCR; // this or the same thing as SP?

    uint8_t* memory;

    uint8_t currentOpcode;
    uint16_t currentPC;

    // Memory Access
    uint8_t GetByte(const uint16_t addr);
    void SetByte(const uint16_t addr, const uint8_t value);
    uint8_t GetNextByte();

    // Instruction Set
    uint8_t UnknownInstruction();
    uint8_t ADC();
    uint8_t ADD();
    uint8_t AND();
    uint8_t ASL();
    uint8_t ASR();
    uint8_t BCC();
    uint8_t BCLR();
    uint8_t BCS();
    uint8_t BEQ();
    uint8_t BHCC();
    uint8_t BHCS();
    uint8_t BHI();
    uint8_t BHS();
    uint8_t BIH();
    uint8_t BIL();
    uint8_t BIT();
    uint8_t BLO();
    uint8_t BLS();
    uint8_t BMC();
    uint8_t BMI();
    uint8_t BMS();
    uint8_t BNE();
    uint8_t BPL();
    uint8_t BRA();
    uint8_t BRN();
    uint8_t BRCLR();
    uint8_t BRSET();
    uint8_t BSET();
    uint8_t BSR();
    uint8_t CLC();
    uint8_t CLI();
    uint8_t CLR();
    uint8_t CMP();
    uint8_t COM();
    uint8_t CPX();
    uint8_t DEC();
    uint8_t EOR();
    uint8_t INC();
    uint8_t JMP();
    uint8_t JSR();
    uint8_t LDA();
    uint8_t LDX();
    uint8_t LSL();
    uint8_t LSR();
    uint8_t MUL();
    uint8_t NEG();
    uint8_t NOP();
    uint8_t ORA();
    uint8_t ROL();
    uint8_t ROR();
    uint8_t RSP();
    uint8_t RTI();
    uint8_t RTS();
    uint8_t SBC();
    uint8_t SEC();
    uint8_t SEI();
    uint8_t STA();
    uint8_t STOP();
    uint8_t STX();
    uint8_t SUB();
    uint8_t SWI();
    uint8_t TAX();
    uint8_t TST();
    uint8_t TXA();
    uint8_t WAIT();

    // Disassembler
    std::string DisassembleUnknownInstruction();
    std::string DisassembleADC();
    std::string DisassembleADD();
    std::string DisassembleAND();
    std::string DisassembleASL();
    std::string DisassembleASR();
    std::string DisassembleBCC();
    std::string DisassembleBCLR();
    std::string DisassembleBCS();
    std::string DisassembleBEQ();
    std::string DisassembleBHCC();
    std::string DisassembleBHCS();
    std::string DisassembleBHI();
    std::string DisassembleBHS();
    std::string DisassembleBIH();
    std::string DisassembleBIL();
    std::string DisassembleBIT();
    std::string DisassembleBLO();
    std::string DisassembleBLS();
    std::string DisassembleBMC();
    std::string DisassembleBMI();
    std::string DisassembleBMS();
    std::string DisassembleBNE();
    std::string DisassembleBPL();
    std::string DisassembleBRA();
    std::string DisassembleBRN();
    std::string DisassembleBRCLR();
    std::string DisassembleBRSET();
    std::string DisassembleBSET();
    std::string DisassembleBSR();
    std::string DisassembleCLC();
    std::string DisassembleCLI();
    std::string DisassembleCLR();
    std::string DisassembleCMP();
    std::string DisassembleCOM();
    std::string DisassembleCPX();
    std::string DisassembleDEC();
    std::string DisassembleEOR();
    std::string DisassembleINC();
    std::string DisassembleJMP();
    std::string DisassembleJSR();
    std::string DisassembleLDA();
    std::string DisassembleLDX();
    std::string DisassembleLSL();
    std::string DisassembleLSR();
    std::string DisassembleMUL();
    std::string DisassembleNEG();
    std::string DisassembleNOP();
    std::string DisassembleORA();
    std::string DisassembleROL();
    std::string DisassembleROR();
    std::string DisassembleRSP();
    std::string DisassembleRTI();
    std::string DisassembleRTS();
    std::string DisassembleSBC();
    std::string DisassembleSEC();
    std::string DisassembleSEI();
    std::string DisassembleSTA();
    std::string DisassembleSTOP();
    std::string DisassembleSTX();
    std::string DisassembleSUB();
    std::string DisassembleSWI();
    std::string DisassembleTAX();
    std::string DisassembleTST();
    std::string DisassembleTXA();
    std::string DisassembleWAIT();

    uint8_t (MC68HC705C8::*ILUT[UINT8_MAX+1])() = {
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
        &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction, &MC68HC705C8::UnknownInstruction,
    };

    std::string (MC68HC705C8::*DLUT[UINT8_MAX+1])() = {
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
        &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction, &MC68HC705C8::DisassembleUnknownInstruction,
    };
};

#endif // MC68HC705C8_HPP
