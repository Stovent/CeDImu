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
    std::mutex onLogDisassemblerMutex;
    std::function<void(const Instruction&)> onLogDisassemblerCallback;

    std::mutex onUARTOutMutex;
    std::function<void(uint8_t)> onUARTOutCallback;

    std::mutex onFrameCompletedMutex;
    std::function<void()> onFrameCompletedCallback;

public:
    explicit Callbacks(const std::function<void(const Instruction&)>& disassembler = nullptr,
                       const std::function<void(uint8_t)>& uartOut = nullptr,
                       const std::function<void()>& frameCompleted = nullptr) :
       onLogDisassemblerCallback(disassembler),
       onUARTOutCallback(uartOut),
       onFrameCompletedCallback(frameCompleted)
   {}

   Callbacks(const Callbacks& other) :
       onLogDisassemblerCallback(other.onLogDisassemblerCallback),
       onUARTOutCallback(other.onUARTOutCallback),
       onFrameCompletedCallback(other.onFrameCompletedCallback)
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
