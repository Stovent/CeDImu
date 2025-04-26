#ifndef CDI_COMMON_CALLBACKS_HPP
#define CDI_COMMON_CALLBACKS_HPP

#include "Video/VideoCommon.hpp"
#include "../cores/SCC68070/SCC68070.hpp"
#include "../OS9/SystemCalls.hpp"

#include <functional>
#include <mutex>

#ifdef ENABLE_LOG
#define LOG(content) content
#else
#define LOG(content)
#endif // ENABLE_LOG

struct LogInstruction
{
    uint32_t address;
    std::string biosLocation;
    std::string instruction;
};

/** \brief The target region of the memory access. */
enum class MemoryAccessLocation
{
    CPU, /**< SCC68070 peripherals. */
    BIOS,
    RAM,
    VDSC,
    Slave,
    CDIC,
    RTC,
    OutOfRange,
};

struct LogMemoryAccess
{
    MemoryAccessLocation location; /**< CPU, BIOS, RAM, VDSC, Slave, RTC, etc. */
    std::string direction; /**< Get or Set. */
    std::string type; /**< Byte, Word, register name, etc. */
    uint32_t pc; /**< Program Counter when the access occured. */
    uint32_t address; /**< The bus address. */
    uint32_t data; /**< The data. */
};

struct LogICADCA
{
    uint32_t frame; /**< The frame number. */
    uint16_t line; /**< The line where the DCA occured, unused by ICA. */
    uint32_t instruction; /**< The instruction. */
};

struct LogSCC68070Exception
{
    uint8_t vector; /**< The vector number. */
    uint32_t returnAddress; /**< The program counter where the CPU will continue after a RTE instruction. */
    std::string disassembled; /**< The disassembled value of the exception. */
    OS9::SystemCall systemCall; /**< If it is a system call, contains its parameters. */
};

/** \brief Class containing the callback functions provided by the user.
 */
class Callbacks
{
    std::mutex onLogDisassemblerMutex{};
    std::function<void(const LogInstruction&)> onLogDisassemblerCallback;

    std::mutex onUARTOutMutex{};
    std::function<void(uint8_t)> onUARTOutCallback;

    std::mutex onFrameCompletedMutex{};
    std::function<void(const Video::Plane&)> onFrameCompletedCallback;

    std::mutex onSaveNVRAMMutex{};
    std::function<void(const void* data, size_t)> onSaveNVRAMCallback;

    std::mutex onLogICADCAMutex{};
    std::function<void(Video::ControlArea, LogICADCA)> onLogICADCACallback;

    std::mutex onLogMemoryAccessMutex{};
    std::function<void(const LogMemoryAccess&)> onLogMemoryAccessCallback;

    std::mutex onLogExceptionMutex{};
    std::function<void(const LogSCC68070Exception&)> onLogExceptionCallback;

    std::mutex onLogRTEMutex{};
    std::function<void(uint32_t, uint16_t)> onLogRTECallback; /**< The parameter is the PC value pulled from the stack. */

public:
    explicit Callbacks(const std::function<void(const LogInstruction&)>& disassembler = nullptr,
                       const std::function<void(uint8_t)>& uartOut = nullptr,
                       const std::function<void(const Video::Plane&)>& frameCompleted = nullptr,
                       const std::function<void(const void* data, size_t)>& saveNVRAM = nullptr,
                       const std::function<void(Video::ControlArea, LogICADCA)>& icadca = nullptr,
                       const std::function<void(const LogMemoryAccess&)>& memoryAccess = nullptr,
                       const std::function<void(const LogSCC68070Exception&)>& logException = nullptr,
                       const std::function<void(uint32_t, uint16_t)>& logRTE = nullptr);

   Callbacks(const Callbacks& other);

    bool HasOnLogDisassembler();
    void SetOnLogDisassembler(const std::function<void(const LogInstruction&)>& callback);
    void OnLogDisassembler(const LogInstruction& arg);

    void SetOnUARTOut(const std::function<void(uint8_t)>& callback);
    void OnUARTOut(uint8_t arg);

    void SetOnFrameCompleted(const std::function<void(const Video::Plane&)>& callback);
    void OnFrameCompleted(const Video::Plane& plane);

    void SetOnSaveNVRAM(const std::function<void(const void*, size_t)>& callback);
    void OnSaveNVRAM(const void* data, size_t size);

    bool HasOnLogICADCA();
    void SetOnLogICADCA(const std::function<void(Video::ControlArea, LogICADCA)>& callback);
    void OnLogICADCA(Video::ControlArea area, LogICADCA inst);

    bool HasOnLogMemoryAccess();
    void SetOnLogMemoryAccess(const std::function<void(const LogMemoryAccess&)>& callback);
    void OnLogMemoryAccess(const LogMemoryAccess& arg);

    bool HasOnLogException();
    void SetOnLogException(const std::function<void(const LogSCC68070Exception&)>& callback);
    void OnLogException(const LogSCC68070Exception& arg);

    bool HasOnLogRTE();
    void SetOnLogRTE(const std::function<void(uint32_t, uint16_t)>& callback);
    void OnLogRTE(uint32_t pc, uint16_t format);
};

/** \brief Returns the MemoryAccessLocation name as a null-terminated string. */
const char* memoryAccessLocationToString(MemoryAccessLocation loc) noexcept;

#endif // CDI_COMMON_CALLBACKS_HPP
