#ifndef CDI_COMMON_CALLBACKS_HPP
#define CDI_COMMON_CALLBACKS_HPP

#include "../cores/SCC68070/SCC68070.hpp"
#include "../cores/VDSC.hpp"
#include "../OS9/SystemCalls.hpp"

#include <functional>
#include <mutex>

#ifdef ENABLE_LOG
#define LOG(content) content
#else
#define LOG(content)
#endif // ENABLE_LOG

/** \struct LogInstruction
 */
struct LogInstruction
{
    uint32_t address;
    std::string biosLocation;
    std::string instruction;
};

enum class MemoryAccessLocation
{
    CPU,
    BIOS,
    RAM,
    VDSC,
    Slave,
    CDIC,
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

/** \struct LogICADCA
 */
struct LogICADCA
{
    uint32_t frame; /**< The frame number. */
    uint16_t line; /**< The line where the DCA occured, unused by ICA. */
    uint32_t instruction; /**< The instruction. */
};

/** \struct LogSCC68070Exception
 */
struct LogSCC68070Exception
{
    uint8_t vector; /**< The vector number. */
    uint32_t returnAddress; /**< The program counter where the CPU will continue after a RTE instruction. */
    std::string disassembled; /**< The disassembled value of the exception. */
    OS9::SystemCall systemCall; /**< If it is a system call, contains its paremeters. */
};

/** \class Callbacks
 * \brief Class containing the callback functions provided by the user.
 */
class Callbacks
{
    std::mutex onLogDisassemblerMutex{};
    std::function<void(const LogInstruction&)> onLogDisassemblerCallback;

    std::mutex onUARTOutMutex{};
    std::function<void(uint8_t)> onUARTOutCallback;

    std::mutex onFrameCompletedMutex{};
    std::function<void(const Plane&)> onFrameCompletedCallback;

    std::mutex onSaveNVRAMMutex{};
    std::function<void(const void* data, size_t)> onSaveNVRAMCallback;

    std::mutex onLogICADCAMutex{};
    std::function<void(ControlArea, LogICADCA)> onLogICADCACallback;

    std::mutex onLogMemoryAccessMutex{};
    std::function<void(const LogMemoryAccess&)> onLogMemoryAccessCallback;

    std::mutex onLogExceptionMutex{};
    std::function<void(const LogSCC68070Exception&)> onLogExceptionCallback;

    std::mutex onLogRTEMutex{};
    std::function<void(uint32_t, uint16_t)> onLogRTECallback; /**< The parameter is the PC value pulled from the stack. */

public:
    explicit Callbacks(const std::function<void(const LogInstruction&)>& disassembler = nullptr,
                       const std::function<void(uint8_t)>& uartOut = nullptr,
                       const std::function<void(const Plane&)>& frameCompleted = nullptr,
                       const std::function<void(const void* data, size_t)>& saveNVRAM = nullptr,
                       const std::function<void(ControlArea, LogICADCA)>& icadca = nullptr,
                       const std::function<void(const LogMemoryAccess&)>& memoryAccess = nullptr,
                       const std::function<void(const LogSCC68070Exception&)>& logException = nullptr,
                       const std::function<void(uint32_t, uint16_t)>& logRTE = nullptr)
       : onLogDisassemblerMutex()
       , onLogDisassemblerCallback(disassembler)
       , onUARTOutMutex()
       , onUARTOutCallback(uartOut)
       , onFrameCompletedMutex()
       , onFrameCompletedCallback(frameCompleted)
       , onSaveNVRAMMutex()
       , onSaveNVRAMCallback(saveNVRAM)
       , onLogICADCAMutex()
       , onLogICADCACallback(icadca)
       , onLogMemoryAccessMutex()
       , onLogMemoryAccessCallback(memoryAccess)
       , onLogExceptionMutex()
       , onLogExceptionCallback(logException)
       , onLogRTEMutex()
       , onLogRTECallback(logRTE)
   {}

   Callbacks(const Callbacks& other)
       : onLogDisassemblerMutex()
       , onLogDisassemblerCallback(other.onLogDisassemblerCallback)
       , onUARTOutMutex()
       , onUARTOutCallback(other.onUARTOutCallback)
       , onFrameCompletedMutex()
       , onFrameCompletedCallback(other.onFrameCompletedCallback)
       , onSaveNVRAMMutex()
       , onSaveNVRAMCallback(other.onSaveNVRAMCallback)
       , onLogICADCAMutex()
       , onLogICADCACallback(other.onLogICADCACallback)
       , onLogMemoryAccessMutex()
       , onLogMemoryAccessCallback(other.onLogMemoryAccessCallback)
       , onLogExceptionMutex()
       , onLogExceptionCallback(other.onLogExceptionCallback)
       , onLogRTEMutex()
       , onLogRTECallback(other.onLogRTECallback)
   {}

   Callbacks(Callbacks&&) = default;

    bool HasOnLogDisassembler()
    {
        std::lock_guard<std::mutex> lock(onLogDisassemblerMutex);
        return (bool)onLogDisassemblerCallback;
    }
    void SetOnLogDisassembler(const std::function<void(const LogInstruction&)>& callback)
    {
        std::lock_guard<std::mutex> lock(onLogDisassemblerMutex);
        onLogDisassemblerCallback = callback;
    }
    void OnLogDisassembler(const LogInstruction& arg)
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
    void SetOnLogICADCA(const std::function<void(ControlArea, LogICADCA)>& callback)
    {
        std::lock_guard<std::mutex> lock(onLogICADCAMutex);
        onLogICADCACallback = callback;
    }
    void OnLogICADCA(ControlArea area, LogICADCA inst)
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

    bool HasOnLogRTE()
    {
        std::lock_guard<std::mutex> lock(onLogRTEMutex);
        return (bool)onLogRTECallback;
    }
    void SetOnLogRTE(const std::function<void(uint32_t, uint16_t)>& callback)
    {
        std::lock_guard<std::mutex> lock(onLogRTEMutex);
        onLogRTECallback = callback;
    }
    void OnLogRTE(uint32_t pc, uint16_t format)
    {
        std::lock_guard<std::mutex> lock(onLogRTEMutex);
        if(onLogRTECallback)
            onLogRTECallback(pc, format);
    }
};

inline const char* memoryAccessLocationToString(const MemoryAccessLocation loc)
{
    switch(loc)
    {
    case MemoryAccessLocation::CPU:   return "CPU";
    case MemoryAccessLocation::BIOS:  return "BIOS";
    case MemoryAccessLocation::RAM:   return "RAM";
    case MemoryAccessLocation::VDSC:  return "VDSC";
    case MemoryAccessLocation::Slave: return "Slave";
    case MemoryAccessLocation::CDIC:  return "CDIC";
    case MemoryAccessLocation::RTC:   return "RTC";
    case MemoryAccessLocation::OutOfRange: return "OUT OF RANGE";
    default: return "Unknown location";
    }
}

#endif // CDI_COMMON_CALLBACKS_HPP
