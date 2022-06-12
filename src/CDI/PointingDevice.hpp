#ifndef CDI_POINTINGDEVICE_HPP
#define CDI_POINTINGDEVICE_HPP

class ISlave;
#include "common/Cycles.hpp"

#include <array>
#include <mutex>

class PointingDevice
{
public:
    enum class Type : char
    {
        Relative = 'M',
        Maneuvering = 'J',
        Absolute = 'T',
        AbsoluteScreen = 'S',
    };

    enum class GamepadSpeed
    {
        N,
        I,
        II,
    };

    ISlave& slave;
    const Type type;
    std::array<uint8_t, 4> pointerMessage{};

    PointingDevice() = delete;
    PointingDevice(ISlave& slv, Type deviceType);

    void IncrementTime(const Cycles& c);

    void SetButton1(const bool pressed);
    void SetButton2(const bool pressed);
    void SetButton12(const bool pressed);
    void SetLeft(const bool pressed);
    void SetUp(const bool pressed);
    void SetRight(const bool pressed);
    void SetDown(const bool pressed);
    void SetAbsolutePointerLocation(const bool pd, const int x, const int y);
    void SetCursorSpeed(const GamepadSpeed speed);

private:
    struct PointerState
    {
        bool btn1; /**< button 1 */
        bool btn2; /**< button 2 */
        bool pd;   /**< pen down */
        int x;     /**< relative x offset or absolute x location */
        int y;     /**< relative y offset or absolute y location */
    };

    Cycles cycles;
    size_t consecutiveCursorPackets; // Ranges from 1 to 8.
    GamepadSpeed gamepadSpeed;

    std::mutex pointerMutex{};
    PointerState pointerState = {false, false, false, 0, 0};
    PointerState lastPointerState = {false, false, false, 0, 0};
    bool padLeft = false;
    bool padUp = false;
    bool padRight = false;
    bool padDown = false;

    void GeneratePointerMessage();
};

/** \brief Returns the number of packets that can be sent per second.
 */
inline size_t getDataPacketFrequency(PointingDevice::Type type)
{
    if(type == PointingDevice::Type::Absolute || type == PointingDevice::Type::AbsoluteScreen)
        return 30;
    return 40;
}

// Gamepad cursor acceleration measured with an oscilloscope:
// speed I only sends 1
// speed N sends 2 2 4 4 6 6 8 8
// speed II sends 2 4 6 8 10 12 14 16
inline int getCursorSpeed(PointingDevice::GamepadSpeed speed, size_t cursorPacketIndex)
{
    switch(speed)
    {
        case PointingDevice::GamepadSpeed::N:  return cursorPacketIndex + (cursorPacketIndex & 1);
        case PointingDevice::GamepadSpeed::I:  return 1;
        case PointingDevice::GamepadSpeed::II: return 2 * cursorPacketIndex;
    }
    return 0;
}

#endif // CDI_POINTINGDEVICE_HPP
