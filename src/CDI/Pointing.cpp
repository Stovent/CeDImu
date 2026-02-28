/** \file Pointing.cpp
 * \brief Implementation of the pointing devices of the CD-I.
 */

#include "Pointing.hpp"
#include "common/utils.hpp"

#include <algorithm>
#include <print>
#include <utility>

namespace Pointing
{

// size_t RelativeCoordinate::GetState(const std::span<uint8_t> packet) noexcept
// {
//     std::unique_lock<std::mutex> lock{m_stateMutex};
//
//     const uint8_t button1 = static_cast<uint8_t>(m_lastState.btn1) << 5;
//     const uint8_t button2 = static_cast<uint8_t>(m_lastState.btn2) << 4;
//
//     const uint8_t x = m_lastState.left ? -SPEED : (m_lastState.right ? SPEED : 0);
//     const uint8_t y = m_lastState.up ? -SPEED : (m_lastState.down ? SPEED : 0);
//
//     std::println("{} {} {} {} {}", SPEED, button1, button2, x, y);
//
//     packet[0] = 0x40 | button1 | button2 | (y >> 4 & 0x0C) | (x >> 6 & 3);
//     packet[1] = x & 0x3F;
//     packet[2] = y & 0x3F;
//
//     return BYTES_PER_PACKET;
// }

// bool RelativeCoordinate::IncrementTime(const double ns) noexcept
// {
//     // TODO: coroutine?
//     bool sendPacket = false;
//     std::lock_guard<std::mutex> lock{m_stateMutex};
//
//     if(!m_timeNs.has_value()) // Packet can be sent immediately.
//     {
//         sendPacket = m_state.HasChanged(m_lastState);
//         if(sendPacket)
//             m_timeNs = 0.0; // Start counting time.
//     }
//     else // m_timeNs has a value, waiting for packet delay.
//     {
//         *m_timeNs += ns;
//
//         if(*m_timeNs >= PACKET_DELTA)
//         {
//             sendPacket = m_state.HasChanged(m_lastState);
//             if(sendPacket)
//                 *m_timeNs -= PACKET_DELTA; // Consecutive packet, start counting again.
//             else
//                 m_timeNs = std::nullopt;
//         }
//     }
//
//     if(sendPacket)
//     {
//         m_lastState = m_state; // To be retrieved later by the Slave chip or driver.
//         m_state.ClearDirections();
//     }
//
//     return sendPacket;
// }

size_t Maneuvering::GetState(const std::span<uint8_t> packet) noexcept
{
    const int speed = GetMovementSpeed();

    const uint8_t button1 = static_cast<uint8_t>(m_lastState.btn1);
    const uint8_t button2 = static_cast<uint8_t>(m_lastState.btn2);

    const uint8_t x = m_lastState.left ? -speed : (m_lastState.right ? speed : 0);
    const uint8_t y = m_lastState.up ? -speed : (m_lastState.down ? speed : 0);

    packet[0] = 0x40 | (button1 << 5) | (button2 << 4) | (y >> 4 & 0x0C) | (x >> 6 & 3);
    packet[1] = x & 0x3F;
    packet[2] = y & 0x3F;

    return Maneuvering::BYTES_PER_PACKET;
}

bool Maneuvering::IncrementTime(const double ns) noexcept
{
    // TODO: coroutine?
    bool sendPacket = false;
    std::lock_guard<std::mutex> lock{m_stateMutex};

    if(!m_timeNs.has_value()) // Packet can be sent immediately.
    {
        sendPacket = m_state.HasAnyButtonChanged(m_lastState) || m_state.HasPadPressed();
        if(sendPacket)
        {
            m_timeNs = 0.0; // Start counting time.
            m_consecutivePackets = 1;
        }
    }
    else // m_timeNs has a value, waiting for packet delay.
    {
        *m_timeNs += ns;

        if(*m_timeNs >= PACKET_DELTA)
        {
            sendPacket = m_state.HasAnyButtonChanged(m_lastState) || m_state.HasPadPressed();
            if(sendPacket) // Consecutive packet, start counting again.
            {
                *m_timeNs -= PACKET_DELTA;
                if(m_state.HasPadPressed())
                    IncrementConsecutivePacket();
                else
                    m_consecutivePackets = 0;
            }
            else // No consecutive packet.
            {
                m_timeNs = std::nullopt;
                m_consecutivePackets = 0;
            }
        }
    }

    if(sendPacket)
        m_lastState = m_state; // To be retrieved later by the Slave chip or driver.

    return sendPacket;
}

int Maneuvering::GetMovementSpeed() const noexcept
{
    switch(m_speed)
    {
    case GamepadSpeed::N:  return std::min(8, m_consecutivePackets + (m_consecutivePackets & 1));
    case GamepadSpeed::I:  return 1;
    case GamepadSpeed::II: return std::min(16, 2 * m_consecutivePackets);
    }
    std::unreachable();
}

} // namespace Pointing
