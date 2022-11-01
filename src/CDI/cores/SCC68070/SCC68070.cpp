#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <cstring>
#include <functional>
#include <iterator>

GetSetResult get_byte(uint32_t addr, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    try {
        return GetSetResult{
            .data = self->GetByte(addr),
            .exception = 0,
        };
    } catch(SCC68070::Exception& ex) {
        return GetSetResult{
            .data = 0,
            .exception = ex.vector,
        };
    }
}

GetSetResult get_word(uint32_t addr, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    uint8_t flags = Trigger;
    if(m68000_scc68070_registers(self->m68000)->pc != addr)
        flags |= Log; // Do not log instruction read.

    try {
        return GetSetResult{
            .data = self->GetWord(addr, flags),
            .exception = 0,
        };
    } catch(SCC68070::Exception& ex) {
        return GetSetResult{
            .data = 0,
            .exception = ex.vector,
        };
    }
}

GetSetResult get_long(uint32_t addr, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    try {
        return GetSetResult{
            .data = self->GetLong(addr),
            .exception = 0,
        };
    } catch(SCC68070::Exception& ex) {
        return GetSetResult{
            .data = 0,
            .exception = ex.vector,
        };
    }
}

GetSetResult set_byte(uint32_t addr, uint8_t data, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    try {
        self->SetByte(addr, data);
        return GetSetResult {0, 0};
    } catch(SCC68070::Exception& ex) {
        return GetSetResult {0, ex.vector};
    }
}

GetSetResult set_word(uint32_t addr, uint16_t data, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    try {
        self->SetWord(addr, data);
        return GetSetResult {0, 0};
    } catch(SCC68070::Exception& ex) {
        return GetSetResult {0, ex.vector};
    }
}

GetSetResult set_long(uint32_t addr, uint32_t data, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    try {
        self->SetLong(addr, data);
        return GetSetResult {0, 0};
    } catch(SCC68070::Exception& ex) {
        return GetSetResult {0, ex.vector};
    }
}

void reset_instruction(void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    self->ResetOperation();
}

/** \brief Build a new SCC68070 CPU.
 *
 * \param idc Reference to the CDI context.
 * \param clockFrequency The frequency of the CPU.
 */
SCC68070::SCC68070(CDI& idc, const uint32_t clockFrequency)
    : currentPC(0)
    , totalCycleCount(0)
    , cdi(idc)
    , executionThread()
    , m68000(m68000_scc68070_new())
    , m68000Callbacks{
        .get_byte = get_byte,
        .get_word = get_word,
        .get_long = get_long,
        .set_byte = set_byte,
        .set_word = set_word,
        .set_long = set_long,
        .reset_instruction = reset_instruction,
        .user_data = this,
    }
    , uartInMutex()
    , uartIn{}
    , loop(false)
    , isRunning(false)
    , cycleDelay((1.0L / clockFrequency) * 1'000'000'000)
    , speedDelay(cycleDelay)
    , timerDelay(cycleDelay * 96)
    , timerCounter(0.0)
    , internal{0}
{
}

/** \brief Destroy the CPU. Stops and wait for the emulation thread to stop if it is running.
 */
SCC68070::~SCC68070()
{
    Stop(true);
    m68000_scc68070_delete(m68000);
}

/** \brief Check if the CPU is running.
 *
 * \return true if it is running, false otherwise.
 */
bool SCC68070::IsRunning() const
{
    return isRunning;
}

/** \brief Set the CPU emulated speed.
 *
 * \param speed The speed multiplier based on the clock frequency used in the constructor.
 *
 * This method only changes the emulation speed, not the clock frequency.
 * A multiplier of 2 will make the CPU runs twice as fast, the GPU to run at twice the framerate,
 * the timekeeper to increment twice as fast, etc.
 */
void SCC68070::SetEmulationSpeed(const double speed)
{
    speedDelay = cycleDelay / speed;
}

/** \brief Start emulation.
 *
 * \param loop If true, will run indefinitely as a thread. If false, will execute a single instruction.
 *
 * If loop = true, executes indefinitely in a thread (non-blocking).
 * If loop = false, executes a single instruction and returns when it is executed (blocking).
 */
void SCC68070::Run(const bool loop)
{
    if(!isRunning)
    {
        if(executionThread.joinable())
            executionThread.join();

        this->loop = loop;
        if(loop)
            executionThread = std::thread(&SCC68070::Interpreter, this);
        else
            Interpreter();
    }
}

/** \brief Stop emulation.
 *
 * \param wait If true, will wait for the thread to join. If false, detach the thread while it stops.
 *
 * If this is invoked during a callback, false must be sent to prevent any dead lock.
 */
void SCC68070::Stop(const bool wait)
{
    loop = false;
    if(executionThread.joinable())
    {
        if(wait)
            executionThread.join();
        else
            executionThread.detach();
    }
}

/** \brief Resets the CPU (as if the RESET and HALT pins are driven LOW).
 */
void SCC68070::Reset()
{
    m68000_scc68070_exception(m68000, ResetSspPc);
    RESET_INTERNAL()
}

/** \brief Trigger interrupt with LIR1 level.
 */
void SCC68070::INT1()
{
    const uint8_t level = internal[LIR] >> 4 & 0x07;
    if(level)
        m68000_scc68070_exception(m68000, static_cast<Vector>(Level1OnChipInterrupt - 1 + level));
}

/** \brief Trigger interrupt with LIR2 level.
 */
void SCC68070::INT2()
{
    const uint8_t level = internal[LIR] & 0x07;
    if(level)
        m68000_scc68070_exception(m68000, static_cast<Vector>(Level1OnChipInterrupt - 1 + level));
}

/** \brief Trigger level 2 external interrupt vector.
 */
void SCC68070::IN2()
{
    m68000_scc68070_exception(m68000, Vector::Level2Interrupt);
}

/** \brief Send a byte through UART.
 * \param byte The byte to send.
 */
void SCC68070::SendUARTIn(const uint8_t byte)
{
    std::lock_guard<std::mutex> lock(uartInMutex);
    uartIn.push_back(byte);
}

/** \brief Get the CPU registers through a const pointer.
 * \return A pointer to the CPU registers.
 */
const Registers* SCC68070::CPURegisters() const
{
    return m68000_scc68070_registers(m68000);
}

/** \brief Get the CPU registers through a mutable pointer.
 * \return A pointer to the CPU registers.
 */
Registers* SCC68070::CPURegisters()
{
    return m68000_scc68070_registers(m68000);
}

/** \brief Get the internal registers.
 *
 * \return A vector containing every internal register with their name, address, value and meaning.
 */
std::vector<InternalRegister> SCC68070::GetInternalRegisters() const
{
    std::vector<InternalRegister> v({
        {"LIR", 0x80001001, internal[LIR], ""},

        {"IDR",  0x80002001, internal[IDR],  ""},
        {"IAR",  0x80002003, internal[IAR],  ""},
        {"ISR",  0x80002005, internal[ISR],  ""},
        {"ICR",  0x80002007, internal[ICR],  ""},
        {"ICCR", 0x80002009, internal[ICCR], ""},

        {"UMR",  0x80002011, internal[UMR],  ""},
        {"USR",  0x80002013, internal[USR],  ""},
        {"UCSR", 0x80002015, internal[UCSR], ""},
        {"UCR",  0x80002017, internal[UCR],  ""},
        {"UTHR", 0x80002019, internal[UTHR], "UART Transmit Holding Register"},
        {"URHR", 0x8000201B, internal[URHR], "UART Receive Holding Register"},

        {"TSR", 0x80002020, internal[TSR], ""},
        {"TCR", 0x80002021, internal[TCR], ""},
        {"RRH", 0x80002022, internal[RRH], ""},
        {"RRL", 0x80002023, internal[RRL], ""},
        {"T0H", 0x80002024, internal[T0H], ""},
        {"T0L", 0x80002025, internal[T0L], ""},
        {"T1H", 0x80002026, internal[T1L], ""},
        {"T1L", 0x80002027, internal[T1L], ""},
        {"T2H", 0x80002028, internal[T2H], ""},
        {"T2L", 0x80002029, internal[T2L], ""},

        {"PICR1", 0x80002045, internal[PICR1], ""},
        {"PICR2", 0x80002047, internal[PICR2], ""},

        {"CSR1",   0x80004000, internal[CSR1],   ""},
        {"CER1",   0x80004001, internal[CER1],   ""},
        {"DCR1",   0x80004004, internal[DCR1],   ""},
        {"OCR1",   0x80004005, internal[OCR1],   ""},
        {"SCR1",   0x80004006, internal[SCR1],   ""},
        {"CCR1",   0x80004007, internal[CCR1],   ""},
        {"MTCH1",  0x8000400A, internal[MTCH1],  ""},
        {"MTCL1",  0x8000400B, internal[MTCL1],  ""},
        {"MACH1",  0x8000400C, internal[MACH1],  ""},
        {"MACMH1", 0x8000400D, internal[MACMH1], ""},
        {"MACML1", 0x8000400E, internal[MACML1], ""},
        {"MACL1",  0x8000400F, internal[MACL1],  ""},
        {"CPR1",   0x8000402B, internal[CPR1],   ""},

        {"CSR2",   0x80004040, internal[CSR2],   ""},
        {"CER2",   0x80004041, internal[CER2],   ""},
        {"DCR2",   0x80004044, internal[DCR2],   ""},
        {"OCR2",   0x80004045, internal[OCR2],   ""},
        {"SCR2",   0x80004046, internal[SCR2],   ""},
        {"CCR2",   0x80004047, internal[CCR2],   ""},
        {"MTCH2",  0x8000404A, internal[MTCH2],  ""},
        {"MTCL2",  0x8000404B, internal[MTCL2],  ""},
        {"MACH2",  0x8000404C, internal[MACH2],  ""},
        {"MACMH2", 0x8000404D, internal[MACMH2], ""},
        {"MACML2", 0x8000404E, internal[MACML2], ""},
        {"MACL2",  0x8000404F, internal[MACL2],  ""},
        {"DACH2",  0x80004054, internal[DACH2],  ""},
        {"DACMH2", 0x80004055, internal[DACMH2], ""},
        {"DACML2", 0x80004056, internal[DACML2], ""},
        {"DACL2",  0x80004057, internal[DACL2],  ""},
        {"CPR2",   0x8000406B, internal[CPR2],   ""},

        {"MSR", 0x80008000, internal[MSR], ""},
        {"MCR", 0x80008001, internal[MCR], ""},
    });

    for(uint8_t i = 0; i < 8; i++)
    {
        v.push_back({"ATTR "         + std::to_string(i), 0x80008040 | (i << 3), (uint16_t)(internal[ATTR + (i << 3)] << 8 | internal[ATTR + (i << 3) + 1]), ""});
        v.push_back({"SEG LENGTH "   + std::to_string(i), 0x80008042 | (i << 3), (uint16_t)(internal[SEG_LENGTH + (i << 3)] << 8 | internal[SEG_LENGTH + (i << 3) + 1]), ""});
        v.push_back({"SEG NUMBER "   + std::to_string(i), 0x80008045 | (i << 3), (uint16_t)(internal[SEG_NUMBER + (i << 3)] << 8), ""});
        v.push_back({"BASE ADDRESS " + std::to_string(i), 0x80008046 | (i << 3), (uint16_t)(internal[BASE_ADDRESS + (i << 3)] << 8 | internal[BASE_ADDRESS + (i << 3) + 1]), ""});
    }

    return v;
}

void SCC68070::ResetOperation()
{
    RESET_INTERNAL()
    cdi.Reset(false);
}

std::string exceptionVectorToString(uint8_t vector)
{
    switch(vector)
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
        default:
            if(vector >= 64)
                return "User interrupt vector " + std::to_string(vector - 64);
            return "Unknown vector " + std::to_string(vector);
    }
}
