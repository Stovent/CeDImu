#include "Callbacks.hpp"

Callbacks::Callbacks(const std::function<void(const LogInstruction&)>& disassembler,
                     const std::function<void(uint8_t)>& uartOut,
                     const std::function<void(const Video::Plane&)>& frameCompleted,
                     const std::function<void(const void* data, size_t)>& saveNVRAM,
                     const std::function<void(Video::ControlArea, LogICADCA)>& icadca,
                     const std::function<void(const LogMemoryAccess&)>& memoryAccess,
                     const std::function<void(const LogSCC68070Exception&)>& logException,
                     const std::function<void(uint32_t, uint16_t)>& logRTE)
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
{
}

Callbacks::Callbacks(const Callbacks& other)
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
{
}

bool Callbacks::HasOnLogDisassembler()
{
    std::lock_guard<std::mutex> lock(onLogDisassemblerMutex);
    return static_cast<bool>(onLogDisassemblerCallback);
}

void Callbacks::SetOnLogDisassembler(const std::function<void(const LogInstruction&)>& callback)
{
    std::lock_guard<std::mutex> lock(onLogDisassemblerMutex);
    onLogDisassemblerCallback = callback;
}

void Callbacks::OnLogDisassembler(const LogInstruction& arg)
{
    std::lock_guard<std::mutex> lock(onLogDisassemblerMutex);
    if(onLogDisassemblerCallback)
        onLogDisassemblerCallback(arg);
}

void Callbacks::SetOnUARTOut(const std::function<void(uint8_t)>& callback)
{
    std::lock_guard<std::mutex> lock(onUARTOutMutex);
    onUARTOutCallback = callback;
}

void Callbacks::OnUARTOut(uint8_t arg)
{
    std::lock_guard<std::mutex> lock(onUARTOutMutex);
    if(onUARTOutCallback)
        onUARTOutCallback(arg);
}

void Callbacks::SetOnFrameCompleted(const std::function<void(const Video::Plane&)>& callback)
{
    std::lock_guard<std::mutex> lock(onFrameCompletedMutex);
    onFrameCompletedCallback = callback;
}

void Callbacks::OnFrameCompleted(const Video::Plane& plane)
{
    std::lock_guard<std::mutex> lock(onFrameCompletedMutex);
    if(onFrameCompletedCallback)
        onFrameCompletedCallback(plane);
}

void Callbacks::SetOnSaveNVRAM(const std::function<void(const void*, size_t)>& callback)
{
    std::lock_guard<std::mutex> lock(onSaveNVRAMMutex);
    onSaveNVRAMCallback = callback;
}

void Callbacks::OnSaveNVRAM(const void* data, size_t size)
{
    std::lock_guard<std::mutex> lock(onSaveNVRAMMutex);
    if(onSaveNVRAMCallback)
        onSaveNVRAMCallback(data, size);
}

bool Callbacks::HasOnLogICADCA()
{
    std::lock_guard<std::mutex> lock(onLogICADCAMutex);
    return static_cast<bool>(onLogICADCACallback);
}

void Callbacks::SetOnLogICADCA(const std::function<void(Video::ControlArea, LogICADCA)>& callback)
{
    std::lock_guard<std::mutex> lock(onLogICADCAMutex);
    onLogICADCACallback = callback;
}

void Callbacks::OnLogICADCA(Video::ControlArea area, LogICADCA inst)
{
    std::lock_guard<std::mutex> lock(onLogICADCAMutex);
    if(onLogICADCACallback)
        onLogICADCACallback(area, inst);
}

bool Callbacks::HasOnLogMemoryAccess()
{
    std::lock_guard<std::mutex> lock(onLogMemoryAccessMutex);
    return static_cast<bool>(onLogMemoryAccessCallback);
}

void Callbacks::SetOnLogMemoryAccess(const std::function<void(const LogMemoryAccess&)>& callback)
{
    std::lock_guard<std::mutex> lock(onLogMemoryAccessMutex);
    onLogMemoryAccessCallback = callback;
}

void Callbacks::OnLogMemoryAccess(const LogMemoryAccess& arg)
{
    std::lock_guard<std::mutex> lock(onLogMemoryAccessMutex);
    if(onLogMemoryAccessCallback)
        onLogMemoryAccessCallback(arg);
}

bool Callbacks::HasOnLogException()
{
    std::lock_guard<std::mutex> lock(onLogExceptionMutex);
    return static_cast<bool>(onLogExceptionCallback);
}

void Callbacks::SetOnLogException(const std::function<void(const LogSCC68070Exception&)>& callback)
{
    std::lock_guard<std::mutex> lock(onLogExceptionMutex);
    onLogExceptionCallback = callback;
}

void Callbacks::OnLogException(const LogSCC68070Exception& arg)
{
    std::lock_guard<std::mutex> lock(onLogExceptionMutex);
    if(onLogExceptionCallback)
        onLogExceptionCallback(arg);
}

bool Callbacks::HasOnLogRTE()
{
    std::lock_guard<std::mutex> lock(onLogRTEMutex);
    return static_cast<bool>(onLogRTECallback);
}

void Callbacks::SetOnLogRTE(const std::function<void(uint32_t, uint16_t)>& callback)
{
    std::lock_guard<std::mutex> lock(onLogRTEMutex);
    onLogRTECallback = callback;
}

void Callbacks::OnLogRTE(uint32_t pc, uint16_t format)
{
    std::lock_guard<std::mutex> lock(onLogRTEMutex);
    if(onLogRTECallback)
        onLogRTECallback(pc, format);
}

const char* memoryAccessLocationToString(const MemoryAccessLocation loc) noexcept
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
