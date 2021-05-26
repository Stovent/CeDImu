#ifndef POINTINGDEVICE_HPP
#define POINTINGDEVICE_HPP

class ISlave;

#include <array>

#define DATA_PACKET_DELAY (25'000'000) // 25ms

enum class PointingDeviceTypes
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
protected:
    size_t timeSinceLastCommand;
    PointerState pointerState = {false, false, false, 0, 0};
    bool padLeft = false;
    bool padUp = false;
    bool padRight = false;
    bool padDown = false;

public:
    ISlave& slave;
    const PointingDeviceTypes type;
    std::array<uint8_t, 4> pointerMessage;

    PointingDevice() = delete;
    PointingDevice(ISlave& slv, const PointingDeviceTypes deviceType) : slave(slv), type(deviceType) {}
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

    void GeneratePointerMessage();
    std::array<uint8_t, 4>::const_iterator Begin() { return pointerMessage.begin(); }
    std::array<uint8_t, 4>::const_iterator End() { auto it = pointerMessage.end(); return (type == PointingDeviceTypes::Absolute || type == PointingDeviceTypes::AbsoluteScreen) ? it : --it; }
};

#endif // POINTINGDEVICE_HPP
