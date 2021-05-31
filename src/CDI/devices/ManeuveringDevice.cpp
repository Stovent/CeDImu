#include "ManeuveringDevice.hpp"
#include "../cores/ISlave.hpp"

ManeuveringDevice::ManeuveringDevice(ISlave& slv) : PointingDevice(slv, PointingDeviceTypes::Maneuvering)
{
    timer = 0;
    cursorSpeed = 8;
}

void ManeuveringDevice::SetCursorSpeed(const GamepadSpeed speed)
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

/** \brief Advance emulated time.
 * \param ns Amount of nanoseconds.
 */
void ManeuveringDevice::IncrementTime(const size_t ns)
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
