#include "PointingDevice.hpp"
#include "../cores/ISlave.hpp"

/** \brief Set button 1 state.
 * \param pressed true if the button is pressed, false otherwise.
 */
void PointingDevice::SetButton1(const bool pressed)
{
    pointerState.btn1 = pressed;
    slave.UpdatePointerState();
}

/** \brief Set button 2 state.
 * \param pressed true if the button is pressed, false otherwise.
 */
void PointingDevice::SetButton2(const bool pressed)
{
    pointerState.btn2 = pressed;
    slave.UpdatePointerState();
}

/** \brief Set pad left state.
 * \param pressed true if left direction is pressed, false otherwise.
 * If left is being pressed while right is already pressed,
 * then right will be unpressed automatically.
 */
void PointingDevice::SetLeft(const bool pressed)
{
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
    pointerState.pd = pd;
    pointerState.x = x;
    pointerState.y = y;
}

// Must be called right before reading the pointer message.
void PointingDevice::GeneratePointerMessage()
{
    switch(type)
    {
    case PointingDeviceTypes::Absolute:
    case PointingDeviceTypes::AbsoluteScreen:
        pointerMessage[0] = 0x40 | pointerState.btn1 << 5 | pointerState.btn2 << 4 | (pointerState.x >> 6 & 0xF);
        pointerMessage[1] = pointerState.pd << 5 | (pointerState.y >> 6 & 0xF);
        pointerMessage[2] = pointerState.x & 0x3F;
        pointerMessage[3] = pointerState.y & 0x3F;
        break;

    case PointingDeviceTypes::Maneuvering:
    case PointingDeviceTypes::Relative:
        pointerMessage[0] = 0x40 | pointerState.btn1 << 5 | pointerState.btn2 << 4 | (pointerState.y >> 4 & 0xC) | (pointerState.x >> 6 & 3);
        pointerMessage[1] = pointerState.x & 0x3F;
        pointerMessage[2] = pointerState.y & 0x3F;
        break;
    }
}
