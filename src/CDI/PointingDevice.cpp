#include "PointingDevice.hpp"
#include "cores/ISlave.hpp"

PointingDevice::PointingDevice(ISlave& slv, const PointingDeviceType deviceType) :
    slave(slv),
    type(deviceType),
    dataPacketDelay((type == PointingDeviceType::Absolute || type == PointingDeviceType::AbsoluteScreen) ? 33'333'333 : 25'000'000),
    timer(0),
    cursorSpeed(8)
{}


void PointingDevice::IncrementTime(const size_t ns)
{
    timer += ns;

    if(timer >= dataPacketDelay)
    {
        std::lock_guard<std::mutex> lock(pointerMutex);
        bool update = false;

        // TODO: handle acceleration and speeds 1, 8, 16
        if(padLeft)
        {
            pointerState.x = -cursorSpeed;
            update = true;
        }
        else if(padRight)
        {
            pointerState.x = cursorSpeed;
            update = true;
        }
        else
            pointerState.x = 0;

        if(padUp)
        {
            pointerState.y = -cursorSpeed;
            update = true;
        }
        else if(padDown)
        {
            pointerState.y = cursorSpeed;
            update = true;
        }
        else
            pointerState.y = 0;

        if(pointerState.btn1 != lastPointerState.btn1)
            update = true;

        if(pointerState.btn2 != lastPointerState.btn2)
            update = true;

        if(update)
        {
            timer = 0;
            GeneratePointerMessage();
            slave.UpdatePointerState();
        }
    }
}

/** \brief Set button 1 state.
 * \param pressed true if the button is pressed, false otherwise.
 */
void PointingDevice::SetButton1(const bool pressed)
{
    std::lock_guard<std::mutex> lock(pointerMutex);
    pointerState.btn1 = pressed;
}

/** \brief Set button 2 state.
 * \param pressed true if the button is pressed, false otherwise.
 */
void PointingDevice::SetButton2(const bool pressed)
{
    std::lock_guard<std::mutex> lock(pointerMutex);
    pointerState.btn2 = pressed;
}

/** \brief Set pad left state.
 * \param pressed true if left direction is pressed, false otherwise.
 * If left is being pressed while right is already pressed,
 * then right will be unpressed automatically.
 */
void PointingDevice::SetLeft(const bool pressed)
{
    std::lock_guard<std::mutex> lock(pointerMutex);
    padLeft = pressed;
    if(pressed && padRight)
        padRight = false;
}

/** \brief Set pad up state.
 * \param pressed true if up direction is pressed, false otherwise.
 * If up is being pressed while down is already pressed,
 * then down will be unpressed automatically.
 */
void PointingDevice::SetUp(const bool pressed)
{
    std::lock_guard<std::mutex> lock(pointerMutex);
    padUp = pressed;
    if(pressed && padDown)
        padDown = false;
}

/** \brief Set pad right state.
 * \param pressed true if right direction is pressed, false otherwise.
 * If right is being pressed while left is already pressed,
 * then left will be unpressed automatically.
 */
void PointingDevice::SetRight(const bool pressed)
{
    std::lock_guard<std::mutex> lock(pointerMutex);
    padRight = pressed;
    if(pressed && padLeft)
        padLeft = false;
}

/** \brief Set pad down state.
 * \param pressed true if down direction is pressed, false otherwise.
 * If down is being pressed while up is already pressed,
 * then up will be unpressed automatically.
 */
void PointingDevice::SetDown(const bool pressed)
{
    std::lock_guard<std::mutex> lock(pointerMutex);
    padDown = pressed;
    if(pressed && padUp)
        padUp = false;
}

/** \brief Set absolute pointer location.
 * \param pd true if pointing indicator on active area, false otherwise.
 * \param x The x coordinate.
 * \param y The y coordinate.
 */
void PointingDevice::SetAbsolutePointerLocation(const bool pd, const int x, const int y)
{
    std::lock_guard<std::mutex> lock(pointerMutex);
    pointerState.pd = pd;
    pointerState.x = x;
    pointerState.y = y;
}

void PointingDevice::SetCursorSpeed(const GamepadSpeed speed)
{
    switch(speed)
    {
    case GamepadSpeed::N:
        cursorSpeed = 8;
        break;

    case GamepadSpeed::I:
        cursorSpeed = 1;
        break;

    case GamepadSpeed::II:
        cursorSpeed = 16;
        break;
    }
}

// pointerLock must be locked before calling this method.
void PointingDevice::GeneratePointerMessage()
{
    switch(type)
    {
    case PointingDeviceType::Absolute:
    case PointingDeviceType::AbsoluteScreen:
        pointerMessage[0] = 0x40 | pointerState.btn1 << 5 | pointerState.btn2 << 4 | (pointerState.x >> 6 & 0xF);
        pointerMessage[1] = pointerState.pd << 5 | (pointerState.y >> 6 & 0xF);
        pointerMessage[2] = pointerState.x & 0x3F;
        pointerMessage[3] = pointerState.y & 0x3F;
        break;

    case PointingDeviceType::Maneuvering:
    case PointingDeviceType::Relative:
        pointerMessage[0] = 0x40 | pointerState.btn1 << 5 | pointerState.btn2 << 4 | (pointerState.y >> 4 & 0xC) | (pointerState.x >> 6 & 3);
        pointerMessage[1] = pointerState.x & 0x3F;
        pointerMessage[2] = pointerState.y & 0x3F;
        pointerMessage[3] = 0;
        break;
    }

    lastPointerState = pointerState;
}
