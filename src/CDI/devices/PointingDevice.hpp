#ifndef POINTINGDEVICE_HPP
#define POINTINGDEVICE_HPP

class ISlave;

#include <array>
#include <mutex>

enum class PointingDeviceTypes : char
{
    Relative = 'M',
    Maneuvering = 'J',
    Absolute = 'T',
    AbsoluteScreen = 'S',
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
    const PointingDeviceTypes type;
    const uint64_t dataPacketDelay;
    std::array<uint8_t, 4> pointerMessage;

    PointingDevice() = delete;
    PointingDevice(ISlave& slv, const PointingDeviceTypes deviceType) : slave(slv), type(deviceType), dataPacketDelay((type == PointingDeviceTypes::Absolute || type == PointingDeviceTypes::AbsoluteScreen) ? 33'000'000 : 25'000'000) {}
    virtual ~PointingDevice() {}

    virtual void IncrementTime(const size_t ns) = 0;

    // TODO: move this in the subclasses?
    void SetButton1(const bool pressed);
    void SetButton2(const bool pressed);
    void SetLeft(const bool pressed);
    void SetUp(const bool pressed);
    void SetRight(const bool pressed);
    void SetDown(const bool pressed);
    void SetAbsolutePointerLocation(const bool pd, const int x, const int y);

    std::array<uint8_t, 4>::const_iterator Begin() { return pointerMessage.begin(); }
    std::array<uint8_t, 4>::const_iterator End() { auto it = pointerMessage.end(); return (type == PointingDeviceTypes::Absolute || type == PointingDeviceTypes::AbsoluteScreen) ? it : --it; }

protected:
    size_t timeSinceLastCommand;
    std::mutex pointerMutex;

    PointerState pointerState = {false, false, false, 0, 0};
    PointerState lastPointerState = {false, false, false, 0, 0};
    bool padLeft = false;
    bool padUp = false;
    bool padRight = false;
    bool padDown = false;

    void GeneratePointerMessage();
};

#endif // POINTINGDEVICE_HPP
