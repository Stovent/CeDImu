#ifndef CDI_COMMON_CALLBACKS_HPP
#define CDI_COMMON_CALLBACKS_HPP

#include "../cores/SCC68070/SCC68070.hpp"
#include "../cores/VDSC.hpp"

#include <functional>
#include <mutex>

#ifdef ENABLE_LOG
#define LOG(content) content
#else
#define LOG(content)
#endif // ENABLE_LOG

enum MemoryAccessLocation
{
    CPU,
    BIOS,
    RAM,
    VDSC,
    Slave,
    RTC,
    OutOfRange,
};

/** \struct LogMemoryAccess
 */
struct LogMemoryAccess
{
    MemoryAccessLocation location; /**< CPU, BIOS, RAM, VDSC, Slave, RTC, etc. */
    std::string direction; /**< Get or Set. */
    std::string type; /**< Byte, Word, register name, etc. */
    uint32_t pc; /**< Program Counter when the access occured. */
    uint32_t address; /**< The bus address. */
    uint32_t data; /**< The data. */
};

enum class ExceptionType
{
    Exception,
    Trap,
    Rte,
};

/** \struct LogSCC68070Exception
 */
struct LogSCC68070Exception
{
    ExceptionType type; /**< Type of the exception. */
    uint32_t returnAddress; /**< The program counter where the CPU will continue after a RTE, or the PC pull in a RTE. */
    uint8_t vector; /**< The vector number. */
    std::string disassembled; /** The disassembled value of the exception. */
};

/** \class Callbacks
 * \brief Class containing the callback functions provided by the user.
 */
class Callbacks
{
    std::mutex onLogDisassemblerMutex;
    std::function<void(const Instruction&)> onLogDisassemblerCallback;

    std::mutex onUARTOutMutex;
    std::function<void(uint8_t)> onUARTOutCallback;

    std::mutex onFrameCompletedMutex;
    std::function<void(const Plane&)> onFrameCompletedCallback;

    std::mutex onSaveNVRAMMutex;
    std::function<void(const void* data, size_t size)> onSaveNVRAMCallback;

    std::mutex onLogICADCAMutex;
    std::function<void(ControlArea, const std::string&)> onLogICADCACallback;

    std::mutex onLogMemoryAccessMutex;
    std::function<void(const LogMemoryAccess&)> onLogMemoryAccessCallback;

    std::mutex onLogExceptionMutex;
    std::function<void(const LogSCC68070Exception&)> onLogExceptionCallback;

public:
    explicit Callbacks(const std::function<void(const Instruction&)>& disassembler = nullptr,
                       const std::function<void(uint8_t)>& uartOut = nullptr,
                       const std::function<void(const Plane&)>& frameCompleted = nullptr,
                       const std::function<void(ControlArea, const std::string&)>& icadca = nullptr,
                       const std::function<void(const LogMemoryAccess&)>& memoryAccess = nullptr,
                       const std::function<void(const LogSCC68070Exception&)>& logException = nullptr) :
       onLogDisassemblerCallback(disassembler),
       onUARTOutCallback(uartOut),
       onFrameCompletedCallback(frameCompleted),
       onLogICADCACallback(icadca),
       onLogMemoryAccessCallback(memoryAccess),
       onLogExceptionCallback(logException)
   {}

   Callbacks(const Callbacks& other) :
       onLogDisassemblerCallback(other.onLogDisassemblerCallback),
       onUARTOutCallback(other.onUARTOutCallback),
       onFrameCompletedCallback(other.onFrameCompletedCallback),
       onLogICADCACallback(other.onLogICADCACallback),
       onLogMemoryAccessCallback(other.onLogMemoryAccessCallback)
   {}

    bool HasOnLogDisassembler()
    {
        std::lock_guard<std::mutex> lock(onLogDisassemblerMutex);
        return (bool)onLogDisassemblerCallback;
    }
    void SetOnLogDisassembler(const std::function<void(const Instruction&)>& callback)
    {
        std::lock_guard<std::mutex> lock(onLogDisassemblerMutex);
        onLogDisassemblerCallback = callback;
    }
    void OnLogDisassembler(const Instruction& arg)
    {
        std::lock_guard<std::mutex> lock(onLogDisassemblerMutex);
        if(onLogDisassemblerCallback)
            onLogDisassemblerCallback(arg);
    }

    void SetOnUARTOut(const std::function<void(uint8_t)>& callback)
    {
        std::lock_guard<std::mutex> lock(onUARTOutMutex);
        onUARTOutCallback = callback;
    }
    void OnUARTOut(uint8_t arg)
    {
        std::lock_guard<std::mutex> lock(onUARTOutMutex);
        if(onUARTOutCallback)
            onUARTOutCallback(arg);
    }

    void SetOnFrameCompleted(const std::function<void(const Plane&)>& callback)
    {
        std::lock_guard<std::mutex> lock(onFrameCompletedMutex);
        onFrameCompletedCallback = callback;
    }
    void OnFrameCompleted(const Plane& plane)
    {
        std::lock_guard<std::mutex> lock(onFrameCompletedMutex);
        if(onFrameCompletedCallback)
            onFrameCompletedCallback(plane);
    }

    void SetOnSaveNVRAM(const std::function<void(const void*, size_t)>& callback)
    {
        std::lock_guard<std::mutex> lock(onSaveNVRAMMutex);
        onSaveNVRAMCallback = callback;
    }
    void OnSaveNVRAM(const void* data, size_t size)
    {
        std::lock_guard<std::mutex> lock(onSaveNVRAMMutex);
        if(onSaveNVRAMCallback)
            onSaveNVRAMCallback(data, size);
    }

    bool HasOnLogICADCA()
    {
        std::lock_guard<std::mutex> lock(onLogICADCAMutex);
        return (bool)onLogICADCACallback;
    }
    void SetOnLogICADCA(const std::function<void(ControlArea, const std::string&)>& callback)
    {
        std::lock_guard<std::mutex> lock(onLogICADCAMutex);
        onLogICADCACallback = callback;
    }
    void OnLogICADCA(ControlArea area, const std::string& inst)
    {
        std::lock_guard<std::mutex> lock(onLogICADCAMutex);
        if(onLogICADCACallback)
            onLogICADCACallback(area, inst);
    }

    bool HasOnLogMemoryAccess()
    {
        std::lock_guard<std::mutex> lock(onLogMemoryAccessMutex);
        return (bool)onLogMemoryAccessCallback;
    }
    void SetOnLogMemoryAccess(const std::function<void(const LogMemoryAccess&)>& callback)
    {
        std::lock_guard<std::mutex> lock(onLogMemoryAccessMutex);
        onLogMemoryAccessCallback = callback;
    }
    void OnLogMemoryAccess(const LogMemoryAccess& arg)
    {
        std::lock_guard<std::mutex> lock(onLogMemoryAccessMutex);
        if(onLogMemoryAccessCallback)
            onLogMemoryAccessCallback(arg);
    }

    bool HasOnLogException()
    {
        std::lock_guard<std::mutex> lock(onLogExceptionMutex);
        return (bool)onLogExceptionCallback;
    }
    void SetOnLogException(const std::function<void(const LogSCC68070Exception&)>& callback)
    {
        std::lock_guard<std::mutex> lock(onLogExceptionMutex);
        onLogExceptionCallback = callback;
    }
    void OnLogException(const LogSCC68070Exception& arg)
    {
        std::lock_guard<std::mutex> lock(onLogExceptionMutex);
        if(onLogExceptionCallback)
            onLogExceptionCallback(arg);
    }
};

inline const char* memoryAccessLocationToString(const MemoryAccessLocation loc)
{
    switch(loc)
    {
    case CPU:   return "CPU";
    case BIOS:  return "BIOS";
    case RAM:   return "RAM";
    case VDSC:  return "VDSC";
    case Slave: return "Slave";
    case RTC:   return "RTC";
    case OutOfRange: return "OUT OF RANGE";
    default:    return "Unknown location";
    }
}

#endif // CDI_COMMON_CALLBACKS_HPP
