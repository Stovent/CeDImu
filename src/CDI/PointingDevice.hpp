#ifndef CDI_POINTINGDEVICE_HPP
#define CDI_POINTINGDEVICE_HPP

class ISlave;

#include <array>
#include <mutex>

enum class PointingDeviceType : char
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

struct PointerState
{
    bool btn1; /**< button 1 */
    bool btn2; /**< button 2 */
    bool pd;   /**< pen down */
    int x;     /**< relative x offset or absolute x location */
    int y;     /**< relative y offset or absolute y location */
};

class PointingDevice
{
public:
    ISlave& slave;
    const PointingDeviceType type;
    const size_t dataPacketDelay;
    std::array<uint8_t, 4> pointerMessage;

    PointingDevice() = delete;
    PointingDevice(ISlave& slv, const PointingDeviceType deviceType);

    void IncrementTime(const size_t ns);

    void SetButton1(const bool pressed);
    void SetButton2(const bool pressed);
    void SetLeft(const bool pressed);
    void SetUp(const bool pressed);
    void SetRight(const bool pressed);
    void SetDown(const bool pressed);
    void SetAbsolutePointerLocation(const bool pd, const int x, const int y);
    void SetCursorSpeed(const GamepadSpeed speed);

private:
    size_t timer;
    size_t consecutiveCursorPackets; // Ranges from 1 to 8.
    GamepadSpeed gamepadSpeed;

    std::mutex pointerMutex;
    PointerState pointerState = {false, false, false, 0, 0};
    PointerState lastPointerState = {false, false, false, 0, 0};
    bool padLeft = false;
    bool padUp = false;
    bool padRight = false;
    bool padDown = false;

    void GeneratePointerMessage();
};

inline size_t getDataPacketDelay(PointingDeviceType type)
{
    if(type == PointingDeviceType::Absolute || type == PointingDeviceType::AbsoluteScreen)
        return 33'333'333;
    return 25'000'000;
}

// Gamepad cursor acceleration measured with an oscilloscope:
// speed I only sends 1
// speed N sends 2 2 4 4 6 6 8 8
// speed II sends 2 4 6 8 10 12 14 16
inline int getCursorSpeed(GamepadSpeed speed, size_t cursorPacketIndex)
{
    switch(speed)
    {
        case GamepadSpeed::N:  return cursorPacketIndex >= 8 ? 8 : cursorPacketIndex + (cursorPacketIndex & 1);
        case GamepadSpeed::I:  return 1;
        case GamepadSpeed::II: return cursorPacketIndex >= 8 ? 16 : 2 * cursorPacketIndex;
    }
    return 0;
}

#endif // CDI_POINTINGDEVICE_HPP
