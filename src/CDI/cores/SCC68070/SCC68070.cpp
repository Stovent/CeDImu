#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <cstring>
#include <iterator>

/** \brief Build a new SCC68070 CPU.
 *
 * \param idc Reference to the CDI context.
 * \param clockFrequency The frequency of the CPU.
 */
SCC68070::SCC68070(CDI& idc, const uint32_t clockFrequency)
    : currentPC(0)
    , totalCycleCount(0)
    , cycleDelay((1.0L / clockFrequency) * 1'000'000'000)
    , m_cdi(idc)
    , m_uartInMutex()
    , m_uartIn{}
    , m_stop(false)
    , m_timerDelay(cycleDelay * 96)
    , m_timerCounter(0)
    , m_peripherals{0}
    , currentOpcode(0)
    , lastAddress(0)
    , D{0}
    , A_{0}
    , PC(0)
    , SR(0)
    , USP(0)
    , SSP(0)
    , m_exceptions{}
    , ILUT(std::make_unique<ILUTFunctionPointer[]>(UINT16_MAX + 1))
    , DLUT(std::make_unique<DLUTFunctionPointer[]>(UINT16_MAX + 1))
{
    GenerateInstructionSet();
}

/** \brief Destroy the CPU.
 */
SCC68070::~SCC68070() noexcept
{
}

/** \brief Resets the CPU (as if the RESET and HALT pins are driven LOW).
 */
void SCC68070::Reset()
{
    ClearExceptions();
    PushException(ResetSSPPC);
    SR |= 0x0700;
    ResetInternal();
}

void SCC68070::ResetInternal()
{
    std::fill(m_peripherals.begin(), m_peripherals.end(), 0);
    SET_TX_READY()
}

void SCC68070::ClearExceptions()
{
    while(m_exceptions.size())
        m_exceptions.pop();
}

/** \brief Requests the CPU to process the given exception.
 * \param ex The exception to process.
 */
void SCC68070::PushException(const Exception& ex)
{
    m_exceptions.emplace(ex);
}

/** \brief Returns the word at the current Program Counter and advances PC by 2.
 * \return The value at PC
 * \warning This method modifies the CPU state. See \sa SCC68070#PeekNextWord.
 */
uint16_t SCC68070::GetNextWord(const BusFlags flags)
{
    uint16_t opcode = GetWord(PC, flags);
    PC += 2;
    return opcode;
}

/** \brief Returns the word at the current Program Counter but does not trigger side effects (TODO).
 * \return The word at PC.
 */
uint16_t SCC68070::PeekNextWord() const noexcept
{
    return m_cdi.PeekWord(PC);
}

/** \brief Trigger interrupt with LIR1 level.
 */
void SCC68070::INT1()
{
    const uint8_t level = m_peripherals[LIR] >> 4 & 0x07;
    if(level != 0)
        PushException(as<ExceptionVector>(Level1OnChipInterruptAutovector - 1 + level));
}

/** \brief Trigger interrupt with LIR2 level.
 */
void SCC68070::INT2()
{
    const uint8_t level = m_peripherals[LIR] & 0x07;
    if(level != 0)
        PushException(as<ExceptionVector>(Level1OnChipInterruptAutovector - 1 + level));
}

/** \brief Trigger level 2 external interrupt vector.
 */
void SCC68070::IN2()
{
    PushException(Level2ExternalInterruptAutovector);
}

/** \brief Send a byte through UART.
 * \param byte The byte to send.
 */
void SCC68070::SendUARTIn(const uint8_t byte)
{
    std::lock_guard<std::mutex> lock(m_uartInMutex);
    m_uartIn.push_back(byte);
}

void SCC68070::AddBreakpoint(const uint32_t address)
{
    m_breakpoints.emplace_back(address);
}

void SCC68070::RemoveBreakpoint(const uint32_t address)
{
    auto it = std::find(m_breakpoints.begin(), m_breakpoints.end(), address);
    if(it != m_breakpoints.end())
        m_breakpoints.erase(it);
}

void SCC68070::ClearAllBreakpoints()
{
    m_breakpoints.clear();
}

/** \brief Set the value of a CPU register.
 *
 * \param reg The register to set.
 * \param value The value to set the register to.
 */
void SCC68070::SetRegister(Register reg, const uint32_t value)
{
    switch(reg)
    {
    case Register::D0: D[0] = value; break;
    case Register::D1: D[1] = value; break;
    case Register::D2: D[2] = value; break;
    case Register::D3: D[3] = value; break;
    case Register::D4: D[4] = value; break;
    case Register::D5: D[5] = value; break;
    case Register::D6: D[6] = value; break;
    case Register::D7: D[7] = value; break;

    case Register::A0: A(0) = value; break;
    case Register::A1: A(1) = value; break;
    case Register::A2: A(2) = value; break;
    case Register::A3: A(3) = value; break;
    case Register::A4: A(4) = value; break;
    case Register::A5: A(5) = value; break;
    case Register::A6: A(6) = value; break;
    case Register::A7: A(7) = value; break;

    case Register::USP: USP = A(7); break;
    case Register::SSP: SSP = A(7); break; // TODO: this is wrong.

    case Register::PC: PC = value; break;
    case Register::SR: SR = value; break;
    }
}

/** \brief Get the CPU registers.
 *
 * \return A map containing the CPU registers with their name and value.
 */
std::map<SCC68070::Register, uint32_t> SCC68070::GetCPURegisters() const
{
    return {
        {Register::D0, D[0]},
        {Register::D1, D[1]},
        {Register::D2, D[2]},
        {Register::D3, D[3]},
        {Register::D4, D[4]},
        {Register::D5, D[5]},
        {Register::D6, D[6]},
        {Register::D7, D[7]},
        {Register::A0, A(0)},
        {Register::A1, A(1)},
        {Register::A2, A(2)},
        {Register::A3, A(3)},
        {Register::A4, A(4)},
        {Register::A5, A(5)},
        {Register::A6, A(6)},
        {Register::A7, A(7)},
        {Register::USP, USP},
        {Register::SSP, SSP},
        {Register::PC, PC},
        {Register::SR, SR},
    };
}

/** \brief Get the internal registers.
 *
 * \return A vector containing every internal register with their name, address, value and meaning.
 */
std::vector<InternalRegister> SCC68070::GetInternalRegisters() const
{
    std::vector<InternalRegister> v({
        {"LIR", 0x80001001, m_peripherals[LIR], ""},

        {"IDR",  0x80002001, m_peripherals[IDR],  ""},
        {"IAR",  0x80002003, m_peripherals[IAR],  ""},
        {"ISR",  0x80002005, m_peripherals[ISR],  ""},
        {"ICR",  0x80002007, m_peripherals[ICR],  ""},
        {"ICCR", 0x80002009, m_peripherals[ICCR], ""},

        {"UMR",  0x80002011, m_peripherals[UMR],  ""},
        {"USR",  0x80002013, m_peripherals[USR],  ""},
        {"UCSR", 0x80002015, m_peripherals[UCSR], ""},
        {"UCR",  0x80002017, m_peripherals[UCR],  ""},
        {"UTHR", 0x80002019, m_peripherals[UTHR], "UART Transmit Holding Register"},
        {"URHR", 0x8000201B, m_peripherals[URHR], "UART Receive Holding Register"},

        {"TSR", 0x80002020, m_peripherals[TSR], ""},
        {"TCR", 0x80002021, m_peripherals[TCR], ""},
        {"RRH", 0x80002022, m_peripherals[RRH], ""},
        {"RRL", 0x80002023, m_peripherals[RRL], ""},
        {"T0H", 0x80002024, m_peripherals[T0H], ""},
        {"T0L", 0x80002025, m_peripherals[T0L], ""},
        {"T1H", 0x80002026, m_peripherals[T1L], ""},
        {"T1L", 0x80002027, m_peripherals[T1L], ""},
        {"T2H", 0x80002028, m_peripherals[T2H], ""},
        {"T2L", 0x80002029, m_peripherals[T2L], ""},

        {"PICR1", 0x80002045, m_peripherals[PICR1], ""},
        {"PICR2", 0x80002047, m_peripherals[PICR2], ""},

        {"CSR1",   0x80004000, m_peripherals[CSR1],   ""},
        {"CER1",   0x80004001, m_peripherals[CER1],   ""},
        {"DCR1",   0x80004004, m_peripherals[DCR1],   ""},
        {"OCR1",   0x80004005, m_peripherals[OCR1],   ""},
        {"SCR1",   0x80004006, m_peripherals[SCR1],   ""},
        {"CCR1",   0x80004007, m_peripherals[CCR1],   ""},
        {"MTCH1",  0x8000400A, m_peripherals[MTCH1],  ""},
        {"MTCL1",  0x8000400B, m_peripherals[MTCL1],  ""},
        {"MACH1",  0x8000400C, m_peripherals[MACH1],  ""},
        {"MACMH1", 0x8000400D, m_peripherals[MACMH1], ""},
        {"MACML1", 0x8000400E, m_peripherals[MACML1], ""},
        {"MACL1",  0x8000400F, m_peripherals[MACL1],  ""},
        {"CPR1",   0x8000402B, m_peripherals[CPR1],   ""},

        {"CSR2",   0x80004040, m_peripherals[CSR2],   ""},
        {"CER2",   0x80004041, m_peripherals[CER2],   ""},
        {"DCR2",   0x80004044, m_peripherals[DCR2],   ""},
        {"OCR2",   0x80004045, m_peripherals[OCR2],   ""},
        {"SCR2",   0x80004046, m_peripherals[SCR2],   ""},
        {"CCR2",   0x80004047, m_peripherals[CCR2],   ""},
        {"MTCH2",  0x8000404A, m_peripherals[MTCH2],  ""},
        {"MTCL2",  0x8000404B, m_peripherals[MTCL2],  ""},
        {"MACH2",  0x8000404C, m_peripherals[MACH2],  ""},
        {"MACMH2", 0x8000404D, m_peripherals[MACMH2], ""},
        {"MACML2", 0x8000404E, m_peripherals[MACML2], ""},
        {"MACL2",  0x8000404F, m_peripherals[MACL2],  ""},
        {"DACH2",  0x80004054, m_peripherals[DACH2],  ""},
        {"DACMH2", 0x80004055, m_peripherals[DACMH2], ""},
        {"DACML2", 0x80004056, m_peripherals[DACML2], ""},
        {"DACL2",  0x80004057, m_peripherals[DACL2],  ""},
        {"CPR2",   0x8000406B, m_peripherals[CPR2],   ""},

        {"MSR", 0x80008000, m_peripherals[MSR], ""},
        {"MCR", 0x80008001, m_peripherals[MCR], ""},
    });

    for(uint8_t i = 0; i < 8; i++)
    {
        v.push_back({"ATTR "         + std::to_string(i), 0x80008040 | (i << 3), as<uint16_t>(m_peripherals[ATTR + (i << 3)] << 8 | m_peripherals[ATTR + (i << 3) + 1]), ""});
        v.push_back({"SEG LENGTH "   + std::to_string(i), 0x80008042 | (i << 3), as<uint16_t>(m_peripherals[SEG_LENGTH + (i << 3)] << 8 | m_peripherals[SEG_LENGTH + (i << 3) + 1]), ""});
        v.push_back({"SEG NUMBER "   + std::to_string(i), 0x80008045 | (i << 3), as<uint16_t>(m_peripherals[SEG_NUMBER + (i << 3)] << 8), ""});
        v.push_back({"BASE ADDRESS " + std::to_string(i), 0x80008046 | (i << 3), as<uint16_t>(m_peripherals[BASE_ADDRESS + (i << 3)] << 8 | m_peripherals[BASE_ADDRESS + (i << 3) + 1]), ""});
    }

    return v;
}

void SCC68070::ResetOperation()
{
    ResetInternal();
    m_cdi.Reset(false);
}

void SCC68070::DumpCPURegisters()
{
    if(!m_cdi.m_callbacks.HasOnLogDisassembler())
        return;

    const std::map<Register, uint32_t>& regs = GetCPURegisters();
    for(const std::pair<Register, uint32_t> reg : regs)
    {
        char s[30];
        snprintf(s, 30, "%s: 0x%08X", CPURegisterToString(reg.first), reg.second);
        m_cdi.m_callbacks.OnLogDisassembler({currentPC, "", s});
    }
}

void SCC68070::SetS(const bool S)
{
    SR &= 0b1101'1111'1111'1111;
    SR |= S << 13;
}

void SCC68070::SetX(const bool X)
{
    SR &= 0b1111'1111'1110'1111;
    SR |= X << 4;
}

void SCC68070::SetN(const bool N)
{
    SR &= 0b1111'1111'1111'0111;
    SR |= N << 3;
}

void SCC68070::SetZ(const bool Z)
{
    SR &= 0b1111'1111'1111'1011;
    SR |= Z << 2;
}

void SCC68070::SetV(const bool V)
{
    SR &= 0b1111'1111'1111'1101;
    SR |= V << 1;
}

void SCC68070::SetC(const bool C)
{
    SR &= 0b1111'1111'1111'1110;
    SR |= C;
}

void SCC68070::SetXC(const bool XC)
{
    SetX(XC);
    SetC(XC);
}

void SCC68070::SetVC(const bool VC)
{
    SetV(VC);
    SetC(VC);
}

void SCC68070::GenerateInstructionOpcodes(const char* format, std::vector<std::vector<int>> values, ILUTFunctionPointer instFunc, DLUTFunctionPointer disFunc)
{
    if(values.size() == 1)
    {
        uint8_t pos = 0;
        uint8_t len = 0;
        for(int i = 0; i < 16; i++)
        {
            if(format[i] != '0' && format[i] != '1')
            {
                char c = format[i];
                pos = i;
                while(format[i] == c)
                {
                    len++;
                    i++;
                }
            }
        }

        for(int value : values[0])
        {
            std::string s(format, pos);
            s += toBinString(value, len);
            if(pos + len < 16)
                s += std::string(format + pos + len, 16 - pos - len);
            ILUT[binStringToInt(s)] = instFunc;
            DLUT[binStringToInt(s)] = disFunc;
        }
    }
    else
    {
        uint8_t pos = 0;
        uint8_t len = 0;
        for(int i = 0; i < 16; i++)
        {
            if(format[i] != '0' && format[i] != '1')
            {
                char c = format[i];
                pos = i;
                while(format[i] == c)
                {
                    len++;
                    i++;
                }
                break;
            }
        }

        for(int value : values[0])
        {
            std::string s(format, pos);
            s += toBinString(value, len);
            if(pos + len < 16)
                s += std::string(format + pos + len, 16 - pos - len);
            GenerateInstructionOpcodes(s.c_str(), std::vector<std::vector<int>>(values.begin()+1, values.end()), instFunc, disFunc);
        }
    }
}

void SCC68070::GenerateInstructionSet()
{
#define FULL_BYTE {\
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,\
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,\
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,\
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,\
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,\
    200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,\
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,\
}

    for(int i = 0; i <= UINT16_MAX; i++) // init LUT with unknown instructions
    {
        ILUT[i] = &SCC68070::UnknownInstruction;
        DLUT[i] = &SCC68070::DisassembleUnknownInstruction;
    }

    GenerateInstructionOpcodes("1100aaa10000bccc", {
        {0, 1, 2, 3, 4, 5, 6, 7}, // aaa
        {0, 1}, // b
        {0, 1, 2, 3, 4, 5, 6, 7}, // ccc
    }, &SCC68070::ABCD, &SCC68070::DisassembleABCD);

    GenerateInstructionOpcodes("1101aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADD, &SCC68070::DisassembleADD);
    GenerateInstructionOpcodes("1101aaabbb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADD, &SCC68070::DisassembleADD);
    GenerateInstructionOpcodes("1101aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::ADD, &SCC68070::DisassembleADD);
    GenerateInstructionOpcodes("1101aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADD, &SCC68070::DisassembleADD);
    GenerateInstructionOpcodes("1101aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},/*effective mode 7*/ {0, 1} }, &SCC68070::ADD, &SCC68070::DisassembleADD);

    GenerateInstructionOpcodes("1101aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADDA, &SCC68070::DisassembleADDA);
    GenerateInstructionOpcodes("1101aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::ADDA, &SCC68070::DisassembleADDA);

    GenerateInstructionOpcodes("00000110aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6},  {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADDI, &SCC68070::DisassembleADDI);
    GenerateInstructionOpcodes("00000110aa111ccc", {{0, 1, 2}, /*effective mode 7*/ {0, 1} }, &SCC68070::ADDI, &SCC68070::DisassembleADDI);

    GenerateInstructionOpcodes("0101aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADDQ, &SCC68070::DisassembleADDQ);
    GenerateInstructionOpcodes("0101aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1} }, &SCC68070::ADDQ, &SCC68070::DisassembleADDQ);
    GenerateInstructionOpcodes("0101aaa0bb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADDQ, &SCC68070::DisassembleADDQ);

    GenerateInstructionOpcodes("1101aaa1bb00cddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ADDX, &SCC68070::DisassembleADDX);

    GenerateInstructionOpcodes("1100aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::AND, &SCC68070::DisassembleAND);
    GenerateInstructionOpcodes("1100aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/* effective mode 7*/{0, 1, 2, 3, 4} }, &SCC68070::AND, &SCC68070::DisassembleAND);
    GenerateInstructionOpcodes("1100aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::AND, &SCC68070::DisassembleAND);
    GenerateInstructionOpcodes("1100aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/* effective mode 7*/{0, 1} }, &SCC68070::AND, &SCC68070::DisassembleAND);

    GenerateInstructionOpcodes("00000010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ANDI, &SCC68070::DisassembleANDI);
    GenerateInstructionOpcodes("00000010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::ANDI, &SCC68070::DisassembleANDI);

    ILUT[0x023C] = &SCC68070::ANDICCR;
    DLUT[0x023C] = &SCC68070::DisassembleANDICCR;

    ILUT[0x027C] = &SCC68070::ANDISR;
    DLUT[0x027C] = &SCC68070::DisassembleANDISR;

    GenerateInstructionOpcodes("1110000a11bbbccc", {{0, 1}, {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ASm, &SCC68070::DisassembleASm);
    GenerateInstructionOpcodes("1110000a11111ccc", {{0, 1}, /* effective mode 7 */ {0, 1} }, &SCC68070::ASm, &SCC68070::DisassembleASm);

    GenerateInstructionOpcodes("1110aaabccd00eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ASr, &SCC68070::DisassembleASr);

    GenerateInstructionOpcodes("0110aaaabbbbbbbb", {{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, FULL_BYTE }, &SCC68070::Bcc, &SCC68070::DisassembleBcc);

    GenerateInstructionOpcodes("0000aaa101bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BCHG, &SCC68070::DisassembleBCHG);
    GenerateInstructionOpcodes("0000aaa101111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} }, &SCC68070::BCHG, &SCC68070::DisassembleBCHG);
    GenerateInstructionOpcodes("0000100001aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BCHG, &SCC68070::DisassembleBCHG);
    ILUT[0x0878] = &SCC68070::BCHG;            ILUT[0x0879] = &SCC68070::BCHG; /*effective mode 7*/
    DLUT[0x0878] = &SCC68070::DisassembleBCHG; DLUT[0x0879] = &SCC68070::DisassembleBCHG;

    GenerateInstructionOpcodes("0000aaa110bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BCLR, &SCC68070::DisassembleBCLR);
    GenerateInstructionOpcodes("0000aaa110111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} }, &SCC68070::BCLR, &SCC68070::DisassembleBCLR);
    GenerateInstructionOpcodes("0000100010aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BCLR, &SCC68070::DisassembleBCLR);
    ILUT[0x08B8] = &SCC68070::BCLR;            ILUT[0x08B9] = &SCC68070::BCLR; /*effective mode 7*/
    DLUT[0x08B8] = &SCC68070::DisassembleBCLR; DLUT[0x08B9] = &SCC68070::DisassembleBCLR;

    GenerateInstructionOpcodes("01100000aaaaaaaa", {FULL_BYTE }, &SCC68070::BRA, &SCC68070::DisassembleBRA);

    GenerateInstructionOpcodes("0000aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BSET, &SCC68070::DisassembleBSET);
    GenerateInstructionOpcodes("0000aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1} }, &SCC68070::BSET, &SCC68070::DisassembleBSET);
    GenerateInstructionOpcodes("0000100011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BSET, &SCC68070::DisassembleBSET);
    ILUT[0x08F8] = &SCC68070::BSET;            ILUT[0x08F9] = &SCC68070::BSET; /*effective mode 7*/
    DLUT[0x08F8] = &SCC68070::DisassembleBSET; DLUT[0x08F9] = &SCC68070::DisassembleBSET;

    GenerateInstructionOpcodes("01100001aaaaaaaa", {FULL_BYTE }, &SCC68070::BSR, &SCC68070::DisassembleBSR);

    GenerateInstructionOpcodes("0000aaa100bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BTST, &SCC68070::DisassembleBTST);
    GenerateInstructionOpcodes("0000aaa100111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::BTST, &SCC68070::DisassembleBTST);
    GenerateInstructionOpcodes("0000100000aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::BTST, &SCC68070::DisassembleBTST);
    ILUT[0x0838] = &SCC68070::BTST; ILUT[0x0839] = &SCC68070::BTST; ILUT[0x083A] = &SCC68070::BTST; ILUT[0x083B] = &SCC68070::BTST; /*effective mode 7*/
    DLUT[0x0838] = &SCC68070::DisassembleBTST;
    DLUT[0x0839] = &SCC68070::DisassembleBTST;
    DLUT[0x083A] = &SCC68070::DisassembleBTST;
    DLUT[0x083B] = &SCC68070::DisassembleBTST;

    GenerateInstructionOpcodes("0100aaa110bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CHK, &SCC68070::DisassembleCHK);
    GenerateInstructionOpcodes("0100aaa110111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::CHK, &SCC68070::DisassembleCHK);

    GenerateInstructionOpcodes("01000010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CLR, &SCC68070::DisassembleCLR);
    GenerateInstructionOpcodes("01000010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::CLR, &SCC68070::DisassembleCLR);

    GenerateInstructionOpcodes("1011aaa000cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, /* byte */ {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CMP, &SCC68070::DisassembleCMP);
    GenerateInstructionOpcodes("1011aaa000111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, /* byte    effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::CMP, &SCC68070::DisassembleCMP);
    GenerateInstructionOpcodes("1011aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CMP, &SCC68070::DisassembleCMP);
    GenerateInstructionOpcodes("1011aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::CMP, &SCC68070::DisassembleCMP);

    GenerateInstructionOpcodes("1011aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CMPA, &SCC68070::DisassembleCMPA);
    GenerateInstructionOpcodes("1011aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::CMPA, &SCC68070::DisassembleCMPA);

    GenerateInstructionOpcodes("00001100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CMPI, &SCC68070::DisassembleCMPI);
    GenerateInstructionOpcodes("00001100aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::CMPI, &SCC68070::DisassembleCMPI);

    GenerateInstructionOpcodes("1011aaa1bb001ccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::CMPM, &SCC68070::DisassembleCMPM);

    GenerateInstructionOpcodes("0101aaaa11001bbb", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::DBcc, &SCC68070::DisassembleDBcc);

    GenerateInstructionOpcodes("1000aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::DIVS, &SCC68070::DisassembleDIVS);
    GenerateInstructionOpcodes("1000aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::DIVS, &SCC68070::DisassembleDIVS);

    GenerateInstructionOpcodes("1000aaa011bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::DIVU, &SCC68070::DisassembleDIVU);
    GenerateInstructionOpcodes("1000aaa011111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::DIVU, &SCC68070::DisassembleDIVU);

    GenerateInstructionOpcodes("1011aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::EOR, &SCC68070::DisassembleEOR);
    GenerateInstructionOpcodes("1011aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::EOR, &SCC68070::DisassembleEOR);

    GenerateInstructionOpcodes("00001010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::EORI, &SCC68070::DisassembleEORI);
    GenerateInstructionOpcodes("00001010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::EORI, &SCC68070::DisassembleEORI);

    ILUT[0x0A3C] = &SCC68070::EORICCR;
    DLUT[0x0A3C] = &SCC68070::DisassembleEORICCR;

    ILUT[0x0A7C] = &SCC68070::EORISR;
    DLUT[0x0A7C] = &SCC68070::DisassembleEORISR;

    GenerateInstructionOpcodes("1100aaa1bbbbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0b01000, 0b01001, 0b10001}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::EXG, &SCC68070::DisassembleEXG);

    GenerateInstructionOpcodes("0100100aaa000bbb", {{0b010, 0b011}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::EXT, &SCC68070::DisassembleEXT);

    ILUT[0x4AFC] = &SCC68070::ILLEGAL;
    DLUT[0x4AFC] = &SCC68070::DisassembleILLEGAL;

    GenerateInstructionOpcodes("0100111011aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::JMP, &SCC68070::DisassembleJMP);
    GenerateInstructionOpcodes("0100111011111bbb", {/*mode 7*/ {0, 1, 2, 3} }, &SCC68070::JMP, &SCC68070::DisassembleJMP);

    GenerateInstructionOpcodes("0100111010aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::JSR, &SCC68070::DisassembleJSR);
    GenerateInstructionOpcodes("0100111010111bbb", {/*mode 7*/ {0, 1, 2, 3} }, &SCC68070::JSR, &SCC68070::DisassembleJSR);

    GenerateInstructionOpcodes("0100aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::LEA, &SCC68070::DisassembleLEA);
    GenerateInstructionOpcodes("0100aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/* mode 7*/ {0, 1, 2, 3} }, &SCC68070::LEA, &SCC68070::DisassembleLEA);

    GenerateInstructionOpcodes("0100111001010aaa", {{0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::LINK, &SCC68070::DisassembleLINK);

    GenerateInstructionOpcodes("1110001a11bbbccc", {{0, 1}, {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::LSm, &SCC68070::DisassembleLSm);
    GenerateInstructionOpcodes("1110001a11111ccc", {{0, 1}, {0, 1} }, &SCC68070::LSm, &SCC68070::DisassembleLSm);

    GenerateInstructionOpcodes("1110aaabccd01eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::LSr, &SCC68070::DisassembleLSr);

    GenerateInstructionOpcodes("00aabbbcccdddeee", {{1, 2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);
    GenerateInstructionOpcodes("00aabbb111dddeee", {{1, 2, 3}, {0, 1}, /* effective mode 7 */ {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);
    GenerateInstructionOpcodes("00aabbbccc111eee", {{1, 2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);
    GenerateInstructionOpcodes("00aabbb111111eee", {{1, 2, 3}, {0, 1}, /* effective mode 7  effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);
    GenerateInstructionOpcodes("00aabbbccc001eee", {{2, 3}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);
    GenerateInstructionOpcodes("00aabbb111001eee", {{2, 3}, {0, 1}, /* effective mode 7 effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVE, &SCC68070::DisassembleMOVE);

    GenerateInstructionOpcodes("001abbb001cccddd", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEA, &SCC68070::DisassembleMOVEA);
    GenerateInstructionOpcodes("001abbb001111ddd", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7}, /* effective mode 7 */ {0, 1, 2, 3, 4} }, &SCC68070::MOVEA, &SCC68070::DisassembleMOVEA);

    GenerateInstructionOpcodes("0100010011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVECCR, &SCC68070::DisassembleMOVECCR);
    GenerateInstructionOpcodes("0100010011111bbb", {/*effective mode 7*/{0, 1, 2, 3, 4} }, &SCC68070::MOVECCR, &SCC68070::DisassembleMOVECCR);

    GenerateInstructionOpcodes("0100000011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEfSR, &SCC68070::DisassembleMOVEfSR);
    ILUT[0x40F8] = &SCC68070::MOVEfSR;            ILUT[0x40F9] = &SCC68070::MOVEfSR;
    DLUT[0x40F8] = &SCC68070::DisassembleMOVEfSR; DLUT[0x40F9] = &SCC68070::DisassembleMOVEfSR;

    GenerateInstructionOpcodes("0100011011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVESR, &SCC68070::DisassembleMOVESR);
    GenerateInstructionOpcodes("0100011011111bbb", {/*effective mode 7*/{0, 1, 2, 3, 4} }, &SCC68070::MOVESR, &SCC68070::DisassembleMOVESR);

    GenerateInstructionOpcodes("010011100110abbb", {{0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEUSP, &SCC68070::DisassembleMOVEUSP);

    GenerateInstructionOpcodes("010010001bcccddd", {{0, 1}, {2, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEM, &SCC68070::DisassembleMOVEM);
    GenerateInstructionOpcodes("010010001b111ddd", {{0, 1}, /* mode 7 */ {0, 1} }, &SCC68070::MOVEM, &SCC68070::DisassembleMOVEM);
    GenerateInstructionOpcodes("010011001bcccddd", {{0, 1}, {2, 3, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEM, &SCC68070::DisassembleMOVEM);
    GenerateInstructionOpcodes("010011001b111ddd", {{0, 1}, /* mode 7 */ {0, 1, 2, 3} }, &SCC68070::MOVEM, &SCC68070::DisassembleMOVEM);

    GenerateInstructionOpcodes("0000aaabbb001ccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6, 7}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MOVEP, &SCC68070::DisassembleMOVEP);

    GenerateInstructionOpcodes("0111aaa0bbbbbbbb", {{0, 1, 2, 3, 4, 5, 6, 7}, FULL_BYTE }, &SCC68070::MOVEQ, &SCC68070::DisassembleMOVEQ);

    GenerateInstructionOpcodes("1100aaa111bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MULS, &SCC68070::DisassembleMULS);
    GenerateInstructionOpcodes("1100aaa111111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::MULS, &SCC68070::DisassembleMULS);

    GenerateInstructionOpcodes("1100aaa011bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::MULU, &SCC68070::DisassembleMULU);
    GenerateInstructionOpcodes("1100aaa011111ccc", {{0, 1, 2, 3, 4, 5, 6, 7},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::MULU, &SCC68070::DisassembleMULU);

    GenerateInstructionOpcodes("0100100000aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::NBCD, &SCC68070::DisassembleNBCD);
    ILUT[0x4838] = &SCC68070::NBCD;            ILUT[0x4839] = &SCC68070::NBCD;
    DLUT[0x4838] = &SCC68070::DisassembleNBCD; DLUT[0x4839] = &SCC68070::DisassembleNBCD;

    GenerateInstructionOpcodes("01000100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::NEG, &SCC68070::DisassembleNEG);
    GenerateInstructionOpcodes("01000100aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::NEG, &SCC68070::DisassembleNEG);

    GenerateInstructionOpcodes("01000000aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::NEGX, &SCC68070::DisassembleNEGX);
    GenerateInstructionOpcodes("01000000aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::NEGX, &SCC68070::DisassembleNEGX);

    ILUT[0x4E71] = &SCC68070::NOP;
    DLUT[0x4E71] = &SCC68070::DisassembleNOP;

    GenerateInstructionOpcodes("01000110aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::NOT, &SCC68070::DisassembleNOT);
    GenerateInstructionOpcodes("01000110aa111ccc", {{0, 1, 2}, {0, 1} }, &SCC68070::NOT, &SCC68070::DisassembleNOT);

    GenerateInstructionOpcodes("1000aaa0bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::OR, &SCC68070::DisassembleOR);
    GenerateInstructionOpcodes("1000aaa0bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::OR, &SCC68070::DisassembleOR);
    GenerateInstructionOpcodes("1000aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::OR, &SCC68070::DisassembleOR);
    GenerateInstructionOpcodes("1000aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::OR, &SCC68070::DisassembleOR);

    GenerateInstructionOpcodes("00000000aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ORI, &SCC68070::DisassembleORI);
    GenerateInstructionOpcodes("00000000aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::ORI, &SCC68070::DisassembleORI);

    ILUT[0x003C] = &SCC68070::ORICCR;
    DLUT[0x003C] = &SCC68070::DisassembleORICCR;

    ILUT[0x007C] = &SCC68070::ORISR;
    DLUT[0x007C] = &SCC68070::DisassembleORISR;

    GenerateInstructionOpcodes("0100100001aaabbb", {{2, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::PEA, &SCC68070::DisassemblePEA);
    GenerateInstructionOpcodes("0100100001111bbb", {/*mode 7*/ {0, 1, 2, 3} }, &SCC68070::PEA, &SCC68070::DisassemblePEA);

    ILUT[0x4E70] = &SCC68070::RESET;
    DLUT[0x4E70] = &SCC68070::DisassembleRESET;

    GenerateInstructionOpcodes("1110011a11bbbccc", {{0, 1},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ROm, &SCC68070::DisassembleROm);
    GenerateInstructionOpcodes("1110011a11111ccc", {{0, 1},/*effective mode 7*/ {0, 1} }, &SCC68070::ROm, &SCC68070::DisassembleROm);

    GenerateInstructionOpcodes("1110aaabccd11eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ROr, &SCC68070::DisassembleROr);

    GenerateInstructionOpcodes("1110010a11bbbccc", {{0, 1},    {2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ROXm, &SCC68070::DisassembleROXm);
    GenerateInstructionOpcodes("1110010a11111ccc", {{0, 1},/*effective mode 7*/ {0, 1} }, &SCC68070::ROXm, &SCC68070::DisassembleROXm);

    GenerateInstructionOpcodes("1110aaabccd10eee", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::ROXr, &SCC68070::DisassembleROXr);

    ILUT[0x4E73] = &SCC68070::RTE;
    DLUT[0x4E73] = &SCC68070::DisassembleRTE;

    ILUT[0x4E77] = &SCC68070::RTR;
    DLUT[0x4E77] = &SCC68070::DisassembleRTR;

    ILUT[0x4E75] = &SCC68070::RTS;
    DLUT[0x4E75] = &SCC68070::DisassembleRTS;

    GenerateInstructionOpcodes("1000aaa10000bccc", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SBCD, &SCC68070::DisassembleSBCD);

    GenerateInstructionOpcodes("0101aaaa11bbbccc", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::Scc, &SCC68070::DisassembleScc);
    GenerateInstructionOpcodes("0101aaaa11111ccc", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},/*effective mode 7*/ {0, 1} }, &SCC68070::Scc, &SCC68070::DisassembleScc);

    ILUT[0x4E72] = &SCC68070::STOP;
    DLUT[0x4E72] = &SCC68070::DisassembleSTOP;

    GenerateInstructionOpcodes("1001aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUB, &SCC68070::DisassembleSUB);
    GenerateInstructionOpcodes("1001aaabbb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, /* effective mode 1 */ {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUB, &SCC68070::DisassembleSUB);
    GenerateInstructionOpcodes("1001aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2},/*effective mode 7*/ {0, 1, 2, 3, 4} }, &SCC68070::SUB, &SCC68070::DisassembleSUB);
    GenerateInstructionOpcodes("1001aaabbbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6}, {   2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUB, &SCC68070::DisassembleSUB);
    GenerateInstructionOpcodes("1001aaabbb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {4, 5, 6},/*effective mode 7*/ {0, 1} }, &SCC68070::SUB, &SCC68070::DisassembleSUB);

    GenerateInstructionOpcodes("1001aaab11cccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUBA, &SCC68070::DisassembleSUBA);
    GenerateInstructionOpcodes("1001aaab11111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1}, {0, 1, 2, 3, 4} }, &SCC68070::SUBA, &SCC68070::DisassembleSUBA);

    GenerateInstructionOpcodes("00000100aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUBI, &SCC68070::DisassembleSUBI);
    GenerateInstructionOpcodes("00000100aa111ccc", {{0, 1, 2}, {0, 1} }, &SCC68070::SUBI, &SCC68070::DisassembleSUBI);

    GenerateInstructionOpcodes("0101aaa1bbcccddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUBQ, &SCC68070::DisassembleSUBQ);
    GenerateInstructionOpcodes("0101aaa1bb111ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1} }, &SCC68070::SUBQ, &SCC68070::DisassembleSUBQ);
    GenerateInstructionOpcodes("0101aaa1bb001ddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {1, 2}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUBQ, &SCC68070::DisassembleSUBQ);

    GenerateInstructionOpcodes("1001aaa1bb00cddd", {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 1, 2}, {0, 1}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SUBX, &SCC68070::DisassembleSUBX);

    GenerateInstructionOpcodes("0100100001000aaa", {{0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::SWAP, &SCC68070::DisassembleSWAP);

    GenerateInstructionOpcodes("0100101011aaabbb", {{0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::TAS, &SCC68070::DisassembleTAS);
    ILUT[0x4AF8] = &SCC68070::TAS;            ILUT[0x4AF9] = &SCC68070::TAS;
    DLUT[0x4AF8] = &SCC68070::DisassembleTAS; DLUT[0x4AF9] = &SCC68070::DisassembleTAS;

    GenerateInstructionOpcodes("010011100100aaaa", {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15} }, &SCC68070::TRAP, &SCC68070::DisassembleTRAP);

    ILUT[0x4E76] = &SCC68070::TRAPV;
    DLUT[0x4E76] = &SCC68070::DisassembleTRAPV;

    GenerateInstructionOpcodes("01001010aabbbccc", {{0, 1, 2}, {0, 2, 3, 4, 5, 6}, {0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::TST, &SCC68070::DisassembleTST);
    GenerateInstructionOpcodes("01001010aa111ccc", {{0, 1, 2},/*effective mode 7*/ {0, 1} }, &SCC68070::TST, &SCC68070::DisassembleTST);

    GenerateInstructionOpcodes("0100111001011aaa", {{0, 1, 2, 3, 4, 5, 6, 7} }, &SCC68070::UNLK, &SCC68070::DisassembleUNLK);
#undef FULL_BYTE
}
