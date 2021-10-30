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
    const uint64_t dataPacketDelay;
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
    uint64_t timer;
    uint32_t cursorSpeed;
    std::mutex pointerMutex;

    PointerState pointerState = {false, false, false, 0, 0};
    PointerState lastPointerState = {false, false, false, 0, 0};
    bool padLeft = false;
    bool padUp = false;
    bool padRight = false;
    bool padDown = false;

    void GeneratePointerMessage();
};

#endif // CDI_POINTINGDEVICE_HPP
