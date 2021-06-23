#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "../cores/SCC68070/SCC68070.hpp"

#include <functional>
#include <mutex>

/** \class Callbacks
 * \brief Class containing the callback functions provided by the user.
*/
class Callbacks
{
    std::mutex onDisassemblerMutex;
    std::function<void(const Instruction&)> onDisassemblerCallback;

    std::mutex onUARTOutMutex;
    std::function<void(uint8_t)> onUARTOutCallback;

    std::mutex onFrameCompletedMutex;
    std::function<void()> onFrameCompletedCallback;

public:
    explicit Callbacks(const std::function<void(const Instruction&)>& disassembler = nullptr,
                       const std::function<void(uint8_t)>& uartOut = nullptr,
                       const std::function<void()>& frameCompleted = nullptr) :
       onDisassemblerCallback(disassembler),
       onUARTOutCallback(uartOut),
       onFrameCompletedCallback(frameCompleted)
   {}

   Callbacks(const Callbacks& other) :
       onDisassemblerCallback(other.onDisassemblerCallback),
       onUARTOutCallback(other.onUARTOutCallback),
       onFrameCompletedCallback(other.onFrameCompletedCallback)
   {}

    bool HasOnDisassembler()
    {
        std::lock_guard<std::mutex> lock(onDisassemblerMutex);
        return (bool)onDisassemblerCallback;
    }
    void SetOnDisassembler(const std::function<void(const Instruction&)>& callback)
    {
        std::lock_guard<std::mutex> lock(onDisassemblerMutex);
        onDisassemblerCallback = callback;
    }
    void OnDisassembler(const Instruction& arg)
    {
        std::lock_guard<std::mutex> lock(onDisassemblerMutex);
        if(onDisassemblerCallback)
            onDisassemblerCallback(arg);
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

    void SetOnFrameCompleted(const std::function<void()>& callback)
    {
        std::lock_guard<std::mutex> lock(onFrameCompletedMutex);
        onFrameCompletedCallback = callback;
    }
    void OnFrameCompleted()
    {
        std::lock_guard<std::mutex> lock(onFrameCompletedMutex);
        if(onFrameCompletedCallback)
            onFrameCompletedCallback();
    }
};

#endif // LOGGER_HPP
