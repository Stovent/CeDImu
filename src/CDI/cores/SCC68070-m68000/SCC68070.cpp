#include "SCC68070-m68000.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <cstring>
#include <iterator>

m68000_memory_result_t getByte(uint32_t addr, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    try
    {
        return m68000_memory_result_t {
            .data = self->GetByte(addr),
            .exception = 0,
        };
    }
    catch(SCC68070::Exception& ex)
    {
        return m68000_memory_result_t {
            .data = 0,
            .exception = static_cast<uint8_t>(ex.vector),
        };
    }
}

m68000_memory_result_t getWord(uint32_t addr, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    const BusFlags flags = (self->m_regs->pc != addr) ? BUS_NORMAL : BUS_INSTRUCTION; // Do not log instruction read.

    try
    {
        return m68000_memory_result_t {
            .data = self->GetWord(addr, flags),
            .exception = 0,
        };
    }
    catch(SCC68070::Exception& ex)
    {
        return m68000_memory_result_t {
            .data = 0,
            .exception = static_cast<uint8_t>(ex.vector),
        };
    }
}

m68000_memory_result_t getLong(uint32_t addr, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    try
    {
        return m68000_memory_result_t {
            .data = self->GetLong(addr),
            .exception = 0,
        };
    }
    catch(SCC68070::Exception& ex)
    {
        return m68000_memory_result_t {
            .data = 0,
            .exception = static_cast<uint8_t>(ex.vector),
        };
    }
}

m68000_memory_result_t setByte(uint32_t addr, uint8_t data, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    try
    {
        self->SetByte(addr, data);
        return m68000_memory_result_t {0, 0};
    }
    catch(SCC68070::Exception& ex)
    {
        return m68000_memory_result_t {0, static_cast<uint8_t>(ex.vector)};
    }
}

m68000_memory_result_t setWord(uint32_t addr, uint16_t data, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    try
    {
        self->SetWord(addr, data);
        return m68000_memory_result_t {0, 0};
    }
    catch(SCC68070::Exception& ex)
    {
        return m68000_memory_result_t {0, static_cast<uint8_t>(ex.vector)};
    }
}

m68000_memory_result_t setLong(uint32_t addr, uint32_t data, void* user_data)
{
    SCC68070* self = static_cast<SCC68070*>(user_data);
    try
    {
        self->SetLong(addr, data);
        return m68000_memory_result_t {0, 0};
    }
    catch(SCC68070::Exception& ex)
    {
        return m68000_memory_result_t {0, static_cast<uint8_t>(ex.vector)};
    }
}

void resetInstruction(void* user_data)
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
    , breakpoints{}
    , m_cdi(idc)
    , m_executionThread()
    , m_uartInMutex()
    , m_uartIn{}
    , m_loop(false)
    , m_isRunning(false)
    , m_m68000{m68000_scc68070_new()}
    , m_memory{
        .get_byte = getByte,
        .get_word = getWord,
        .get_long = getLong,
        .set_byte = setByte,
        .set_word = setWord,
        .set_long = setLong,
        .reset_instruction = resetInstruction,
        .user_data = this,
    }
    , m_regs(m68000_scc68070_registers(m_m68000.get()))
    , m_cycleDelay((1.0L / clockFrequency) * 1'000'000'000)
    , m_speedDelay(m_cycleDelay)
    , m_timerDelay(m_cycleDelay * 96)
    , m_timerCounter(0)
    , m_peripherals{0}
{
    if(m_m68000 == nullptr)
        throw std::runtime_error("Failed to create SCC68070 CPU.");
}

/** \brief Destroy the CPU. Stops and wait for the emulation thread to stop if it is running.
 */
SCC68070::~SCC68070()
{
    Stop(true);
}

/** \brief Check if the CPU is running.
 *
 * \return true if it is running, false otherwise.
 */
bool SCC68070::IsRunning() const
{
    return m_isRunning;
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
    m_speedDelay = m_cycleDelay / speed;
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
    if(!m_isRunning)
    {
        if(m_executionThread.joinable())
            m_executionThread.join();

        m_loop = loop;
        if(loop)
            m_executionThread = std::thread(&SCC68070::Interpreter, this);
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
    m_loop = false;
    if(m_executionThread.joinable())
    {
        if(wait)
            m_executionThread.join();
        else
            m_executionThread.detach();
    }
}

/** \brief Resets the CPU (as if the RESET and HALT pins are driven LOW).
 */
void SCC68070::Reset()
{
    m68000_scc68070_exception(m_m68000.get(), ResetSspPc);
    ResetInternal();
}

void SCC68070::ResetInternal()
{
    std::fill(m_peripherals.begin(), m_peripherals.end(), 0);
    SET_TX_READY()
}

/** \brief Trigger interrupt with LIR1 level.
 */
void SCC68070::INT1()
{
    const uint8_t level = m_peripherals[LIR] >> 4 & 0x07;
    if(level != 0)
        m68000_scc68070_exception(m_m68000.get(), as<m68000_vector_t>(Level1OnChipInterrupt - 1 + level));
}

/** \brief Trigger interrupt with LIR2 level.
 */
void SCC68070::INT2()
{
    const uint8_t level = m_peripherals[LIR] & 0x07;
    if(level != 0)
        m68000_scc68070_exception(m_m68000.get(), as<m68000_vector_t>(Level1OnChipInterrupt - 1 + level));
}

/** \brief Trigger level 2 external interrupt vector.
 */
void SCC68070::IN2()
{
    m68000_scc68070_exception(m_m68000.get(), Level2Interrupt);
}

/** \brief Send a byte through UART.
 * \param byte The byte to send.
 */
void SCC68070::SendUARTIn(const uint8_t byte)
{
    std::lock_guard<std::mutex> lock(m_uartInMutex);
    m_uartIn.push_back(byte);
}

// /** \brief Set the value of a CPU register.
//  *
//  * \param reg The register to set.
//  * \param value The value to set the register to.
//  */
// void SCC68070::SetRegister(Register reg, const uint32_t value)
// {
//     switch(reg)
//     {
//     case Register::D0: D[0] = value; break;
//     case Register::D1: D[1] = value; break;
//     case Register::D2: D[2] = value; break;
//     case Register::D3: D[3] = value; break;
//     case Register::D4: D[4] = value; break;
//     case Register::D5: D[5] = value; break;
//     case Register::D6: D[6] = value; break;
//     case Register::D7: D[7] = value; break;
//
//     case Register::A0: A(0) = value; break;
//     case Register::A1: A(1) = value; break;
//     case Register::A2: A(2) = value; break;
//     case Register::A3: A(3) = value; break;
//     case Register::A4: A(4) = value; break;
//     case Register::A5: A(5) = value; break;
//     case Register::A6: A(6) = value; break;
//     case Register::A7: A(7) = value; break;
//
//     case Register::USP: USP = A(7); break;
//     case Register::SSP: SSP = A(7); break; // TODO: this is wrong.
//
//     case Register::PC: PC = value; break;
//     case Register::SR: SR = value; break;
//     }
// }

static constexpr uint16_t packSR(const m68000_status_register_t& sr) noexcept
{
    return (sr.t << 15) | (sr.s << 13) | (sr.interrupt_mask << 8) | (sr.x << 4) | (sr.n << 3) | (sr.z << 2) | (sr.v << 1) | sr.c;
}

/** \brief Get the CPU registers.
 *
 * \return A map containing the CPU registers with their name and value.
 */
std::map<SCC68070::Register, uint32_t> SCC68070::GetCPURegisters() const
{
    return {
        {Register::D0, m_regs->d[0]},
        {Register::D1, m_regs->d[1]},
        {Register::D2, m_regs->d[2]},
        {Register::D3, m_regs->d[3]},
        {Register::D4, m_regs->d[4]},
        {Register::D5, m_regs->d[5]},
        {Register::D6, m_regs->d[6]},
        {Register::D7, m_regs->d[7]},
        {Register::A0, m_regs->a[0]},
        {Register::A1, m_regs->a[1]},
        {Register::A2, m_regs->a[2]},
        {Register::A3, m_regs->a[3]},
        {Register::A4, m_regs->a[4]},
        {Register::A5, m_regs->a[5]},
        {Register::A6, m_regs->a[6]},
        {Register::A7, m_regs->sr.s ? m_regs->ssp : m_regs->usp},
        {Register::USP, m_regs->usp},
        {Register::SSP, m_regs->ssp},
        {Register::PC, m_regs->pc},
        {Register::SR, packSR(m_regs->sr)},
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

// void SCC68070::DumpCPURegisters()
// {
//     if(!m_cdi.m_callbacks.HasOnLogDisassembler())
//         return;
//
//     const std::map<Register, uint32_t>& regs = GetCPURegisters();
//     for(const std::pair<Register, uint32_t> reg : regs)
//     {
//         char s[30];
//         snprintf(s, 30, "%s: 0x%08X", CPURegisterToString(reg.first), reg.second);
//         m_cdi.m_callbacks.OnLogDisassembler({currentPC, "", s});
//     }
// }
