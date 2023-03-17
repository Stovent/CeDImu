#ifndef CDI_POINTINGDEVICE_HPP
#define CDI_POINTINGDEVICE_HPP

class ISlave;

#include <array>
#include <mutex>

class PointingDevice
{
public:
    enum class Class : char
    {
        Relative = 'M',
        Maneuvering = 'J',
        AbsoluteCoordinate = 'T',
        AbsoluteScreen = 'S',
    };

    enum class GamepadSpeed
    {
        N,
        I,
        II,
    };

    ISlave& m_slave;
    const Class m_deviceClass;
    const size_t m_dataPacketDelay;
    std::array<uint8_t, 4> m_pointerMessage{};

    PointingDevice() = delete;
    PointingDevice(ISlave& slv, Class deviceClass);

    void IncrementTime(const size_t ns);

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

    size_t m_timer;
    int m_consecutiveCursorPackets; // Ranges from 1 to 8.
    GamepadSpeed m_gamepadSpeed;

    int GetCursorSpeed() const;

    std::mutex m_pointerMutex{};
    PointerState m_pointerState{false, false, false, 0, 0};
    PointerState m_lastPointerState{false, false, false, 0, 0};
    bool m_padLeft = false;
    bool m_padUp = false;
    bool m_padRight = false;
    bool m_padDown = false;

    void GeneratePointerMessage();
};

#endif // CDI_POINTINGDEVICE_HPP
