#include "PointingDevice.hpp"
#include "cores/ISlave.hpp"

#include <algorithm>

static constexpr inline size_t getDataPacketDelay(PointingDevice::Class type)
{
    if(type == PointingDevice::Class::AbsoluteCoordinate || type == PointingDevice::Class::AbsoluteScreen)
        return 33'333'333;
    return 25'000'000;
}

PointingDevice::PointingDevice(ISlave& slv, PointingDevice::Class deviceClass)
    : m_slave(slv)
    , m_deviceClass(deviceClass)
    , m_dataPacketDelay(getDataPacketDelay(deviceClass))
    , m_timer(0)
    , m_consecutiveCursorPackets(0)
    , m_gamepadSpeed(GamepadSpeed::N)
{}

void PointingDevice::IncrementTime(const size_t ns)
{
    m_timer += ns;

    if(m_timer >= m_dataPacketDelay)
    {
        m_timer -= m_dataPacketDelay;
        bool update = false;
        std::lock_guard<std::mutex> lock(m_pointerMutex);

        if(!m_padLeft && !m_padUp && !m_padRight && !m_padDown)
            m_consecutiveCursorPackets = 0;
        else
            m_consecutiveCursorPackets++;

        const int speed = GetCursorSpeed();

        if(m_padLeft)
        {
            m_pointerState.x = -speed;
            update = true;
            if(m_lastPointerState.x < m_pointerState.x)
                m_consecutiveCursorPackets = 0;
        }
        else if(m_padRight)
        {
            m_pointerState.x = speed;
            update = true;
            if(m_lastPointerState.x > m_pointerState.x)
                m_consecutiveCursorPackets = 0;
        }
        else
            m_pointerState.x = 0;

        if(m_padUp)
        {
            m_pointerState.y = -speed;
            update = true;
            if(m_lastPointerState.y < m_pointerState.y)
                m_consecutiveCursorPackets = 0;
        }
        else if(m_padDown)
        {
            m_pointerState.y = speed;
            update = true;
            if(m_lastPointerState.y > m_pointerState.y)
                m_consecutiveCursorPackets = 0;
        }
        else
            m_pointerState.y = 0;

        if(m_pointerState.btn1 != m_lastPointerState.btn1)
            update = true;

        if(m_pointerState.btn2 != m_lastPointerState.btn2)
            update = true;

        if(update)
        {
            GeneratePointerMessage();
            m_slave.UpdatePointerState();
        }
    }
}

/** \brief Set button 1 state.
 * \param pressed true if the button is pressed, false otherwise.
 */
void PointingDevice::SetButton1(const bool pressed)
{
    std::lock_guard<std::mutex> lock(m_pointerMutex);
    m_pointerState.btn1 = pressed;
}

/** \brief Set button 2 state.
 * \param pressed true if the button is pressed, false otherwise.
 */
void PointingDevice::SetButton2(const bool pressed)
{
    std::lock_guard<std::mutex> lock(m_pointerMutex);
    m_pointerState.btn2 = pressed;
}

/** \brief Set both the button 1 and 2 state.
 * \param pressed true if the buttons are pressed, false otherwise.
 */
void PointingDevice::SetButton12(const bool pressed)
{
    SetButton1(pressed);
    SetButton2(pressed);
}

/** \brief Set pad left state.
 * \param pressed true if left direction is pressed, false otherwise.
 * If left is being pressed while right is already pressed,
 * then right will be unpressed automatically.
 */
void PointingDevice::SetLeft(const bool pressed)
{
    std::lock_guard<std::mutex> lock(m_pointerMutex);
    m_padLeft = pressed;
    if(pressed && m_padRight)
        m_padRight = false;
}

/** \brief Set pad up state.
 * \param pressed true if up direction is pressed, false otherwise.
 * If up is being pressed while down is already pressed,
 * then down will be unpressed automatically.
 */
void PointingDevice::SetUp(const bool pressed)
{
    std::lock_guard<std::mutex> lock(m_pointerMutex);
    m_padUp = pressed;
    if(pressed && m_padDown)
        m_padDown = false;
}

/** \brief Set pad right state.
 * \param pressed true if right direction is pressed, false otherwise.
 * If right is being pressed while left is already pressed,
 * then left will be unpressed automatically.
 */
void PointingDevice::SetRight(const bool pressed)
{
    std::lock_guard<std::mutex> lock(m_pointerMutex);
    m_padRight = pressed;
    if(pressed && m_padLeft)
        m_padLeft = false;
}

/** \brief Set pad down state.
 * \param pressed true if down direction is pressed, false otherwise.
 * If down is being pressed while up is already pressed,
 * then up will be unpressed automatically.
 */
void PointingDevice::SetDown(const bool pressed)
{
    std::lock_guard<std::mutex> lock(m_pointerMutex);
    m_padDown = pressed;
    if(pressed && m_padUp)
        m_padUp = false;
}

/** \brief Set absolute pointer location.
 * \param pd true if pointing indicator on active area, false otherwise.
 * \param x The x coordinate.
 * \param y The y coordinate.
 */
void PointingDevice::SetAbsolutePointerLocation(const bool pd, const int x, const int y)
{
    std::lock_guard<std::mutex> lock(m_pointerMutex);
    m_pointerState.pd = pd;
    m_pointerState.x = x;
    m_pointerState.y = y;
}

void PointingDevice::SetCursorSpeed(const GamepadSpeed speed)
{
    std::lock_guard<std::mutex> lock(m_pointerMutex);
    m_gamepadSpeed = speed;
}

// Gamepad cursor acceleration measured with an oscilloscope:
// speed I only sends 1
// speed N sends 2 2 4 4 6 6 8 8
// speed II sends 2 4 6 8 10 12 14 16
int PointingDevice::GetCursorSpeed() const
{
    switch(m_gamepadSpeed)
    {
    case GamepadSpeed::N:  return std::min(8, m_consecutiveCursorPackets + (m_consecutiveCursorPackets & 1));
    case GamepadSpeed::I:  return 1;
    case GamepadSpeed::II: return std::min(16, 2 * m_consecutiveCursorPackets);
    }
    // std::unreachable();
    return 0;
}

// m_pointerMutex must be locked before calling this method.
void PointingDevice::GeneratePointerMessage()
{
    switch(m_deviceClass)
    {
    case Class::AbsoluteCoordinate:
    case Class::AbsoluteScreen:
        m_pointerMessage[0] = 0x40 | m_pointerState.btn1 << 5 | m_pointerState.btn2 << 4 | (m_pointerState.x >> 6 & 0xF);
        m_pointerMessage[1] = m_pointerState.pd << 5 | (m_pointerState.y >> 6 & 0xF);
        m_pointerMessage[2] = m_pointerState.x & 0x3F;
        m_pointerMessage[3] = m_pointerState.y & 0x3F;
        break;

    case Class::Maneuvering:
    case Class::Relative:
        m_pointerMessage[0] = 0x40 | m_pointerState.btn1 << 5 | m_pointerState.btn2 << 4 | (m_pointerState.y >> 4 & 0xC) | (m_pointerState.x >> 6 & 3);
        m_pointerMessage[1] = m_pointerState.x & 0x3F;
        m_pointerMessage[2] = m_pointerState.y & 0x3F;
        m_pointerMessage[3] = 0;
        break;
    }

    m_lastPointerState = m_pointerState;
}
