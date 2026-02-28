/** \file Pointing.hpp
 * \brief Implementation of the pointing devices of the CD-I.
 * \todo Make it a Pointing Directory with one header/impl per device class?
 *
 * The idea is that each device has its own capabilities, but the only thing that the software and hardware sees are the
 * packets, common to all pointing devices. See `http://icdia.co.uk/docs/pointing_devices.pdf`.
 */

#ifndef CDI_POINTING_HPP
#define CDI_POINTING_HPP

#include <algorithm>
#include <mutex>
#include <optional>
#include <span>

/** \brief Pointing devices. */
namespace Pointing
{

/** \brief Abstraction of all the pointing devices. */
class Device
{
public:
    /** \brief The pointing device type. */
    enum class Type : char
    {
        Relative = 'M',
        Maneuvering = 'J',
        AbsoluteCoordinate = 'T',
        AbsoluteScreen = 'S',
    };

    Device() noexcept = default;
    virtual ~Device() noexcept = default;

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&&) = delete;
    Device& operator=(Device&&) = delete;

    /** \brief Writes the 3 or 4 bytes packet to the given buffer.
     * \param packets The destination packet buffer, must hold at least 4 bytes.
     * \return The number of bytes written.
     */
    // TODO: return inplace_vector?
    // TODO: return optional<inplace_vector> from IncrementTime?
    virtual size_t GetState(std::span<uint8_t> packets) noexcept = 0;

    /** \brief Increment the emulated time.
     * \param ns The time in nanoseconds.
     * \return true when a new packet is sent entirely and can be read from GetState.
     */
    virtual bool IncrementTime(double ns) noexcept = 0;
};

// /** \brief Implementation of relative coordinate devices. */
// class RelativeCoordinate : public Device
// {
// public:
//     static constexpr Device::Type DEVICE_TYPE = Device::Type::Relative;
//     static constexpr double PACKET_DELTA = 25'000'000.0; /**< The delta in ns between two packets. */
//     static constexpr size_t BYTES_PER_PACKET = 3; /**< The number of bytes per packet sent. */
//     static constexpr int SPEED = 8; /**< The default speed of the device (in pixels per packet). */
//
//     explicit RelativeCoordinate() noexcept
//         : Device{}
//     {}
//
//     virtual size_t GetState(std::span<uint8_t> packet) noexcept override;
//
//     virtual bool IncrementTime(double ns) noexcept override;
//
// #define LOCK std::unique_lock<std::mutex> m{m_stateMutex}
//
//     void SetButton1(const bool pressed) noexcept { LOCK; m_state.btn1 = pressed; }
//     void SetButton2(const bool pressed) noexcept { LOCK; m_state.btn2 = pressed; }
//     void SetUp(const bool pressed) noexcept { LOCK; m_state.up = pressed; }
//     void SetRight(const bool pressed) noexcept { LOCK; m_state.right = pressed; }
//     void SetDown(const bool pressed) noexcept { LOCK; m_state.down = pressed; }
//     void SetLeft(const bool pressed) noexcept { LOCK; m_state.left = pressed; }
//
// #undef LOCK
// private:
//     /** \brief Relative coordinate state, where directions reset between packets. */
//     struct State
//     {
//         bool up{}; /**< Has moved up. */
//         bool right{}; /**< Has moved right. */
//         bool down{}; /**< Has moved down. */
//         bool left{}; /**< Has moved left. */
//         bool btn1{}; /**< button 1. */
//         bool btn2{}; /**< button 2. */
//
//         // /** \brief Checks if the state is changed. */
//         // constexpr bool HasChanged() const noexcept { return btn1 | btn2 | up | right | down | left; }
//         // /** \brief Checks if any of the pad buttons are pressed. */
//         // constexpr bool HasPadButtonPressed() const noexcept { return up | right | down | left; }
//
//         /** \brief Checks if any button has changed state since the last packet. */
//         constexpr bool HasChanged(const State& previous) const noexcept
//         {
//             return (btn1 != previous.btn1) || (btn2 != previous.btn2) || up || right || down || left;
//         }
//
//         // /** \brief Checks if the pad has changed state since the last packet. */
//         // constexpr bool HasPadChanged(const State& other) const noexcept
//         // {
//         //     return (up != other.up) || (right != other.right) || (down != other.down) || (left != other.left);
//         // }
//
//         /** \brief Sets all the directions to unmoved. */
//         constexpr void ClearDirections() noexcept { up = right = down = left = false; }
//     };
//
//     std::optional<double> m_timeNs{}; /**< std::nullopt means a new packet can be sent, otherwise increments until delay is reached. */
//
//     std::mutex m_stateMutex{}; /**< Mutex to hold when accessing \ref m_pointerState. */
//     State m_state{}; /**< Holds the state between packets. Acquire \ref m_stateMutex first. */
//     State m_lastState{}; /**< Holds the state to return when GetState() is called, after a packet has been sent. */
// };

/** \brief Implementation of maneuvering devices with multiple speeds.
 * The gamepad has a physical speed setting and moves with a accelerating movement.
 * The acceleration sends the following values (positive or negative) when holding the pad:
 * - speed I only sends 1 (max speed 1)
 * - speed N sends 2 2 4 4 6 6 8 8 (max speed 8)
 * - speed II sends 2 4 6 8 10 12 14 16 (max speed 16)
 *
 * \todo: I am not sure how acceleration precisely works. More importantly, is it per axis? per direction? one for all directions?
 */
class Maneuvering : public Device
{
public:
    enum class GamepadSpeed
    {
        N,
        I,
        II,
    };

    static constexpr Device::Type DEVICE_TYPE = Device::Type::Maneuvering;
    static constexpr double PACKET_DELTA = 25'000'000.0; /**< The delta in ns between two packets. */
    static constexpr size_t BYTES_PER_PACKET = 3; /**< The number of bytes per packet sent. */

    explicit Maneuvering(GamepadSpeed speed = GamepadSpeed::N) noexcept
        : Device{}
        , m_speed{speed}
    {}

    virtual size_t GetState(std::span<uint8_t> packet) noexcept override;

    virtual bool IncrementTime(double ns) noexcept override;

#define LOCK std::unique_lock<std::mutex> m{m_stateMutex}

    void SetSpeed(const GamepadSpeed speed) noexcept { m_speed = speed; }
    void SetUp(const bool pressed) noexcept { LOCK; m_state.up = pressed; }
    void SetRight(const bool pressed) noexcept { LOCK; m_state.right = pressed; }
    void SetDown(const bool pressed) noexcept { LOCK; m_state.down = pressed; }
    void SetLeft(const bool pressed) noexcept { LOCK; m_state.left = pressed; }
    void SetButton1(const bool pressed) noexcept { LOCK; m_state.btn1 = pressed; }
    void SetButton2(const bool pressed) noexcept { LOCK; m_state.btn2 = pressed; }
    void SetButton12(const bool pressed) noexcept { LOCK; m_state.btn1 = m_state.btn2 = pressed; }

#undef LOCK
private:
    /** \brief Holds the state of the button (pressed or unpressed). */
    struct State
    {
        bool up{}; /**< Pad up. */
        bool right{}; /**< Pad right. */
        bool down{}; /**< Pad down. */
        bool left{}; /**< Pad left. */
        bool btn1{}; /**< button 1. */
        bool btn2{}; /**< button 2. */

        /** \brief Checks if any of the pad buttons are pressed. */
        constexpr bool HasPadPressed() const noexcept { return up || right || down || left; }

        /** \brief Checks if any button has changed state since the last packet. */
        constexpr bool HasAnyButtonChanged(const State& other) const noexcept
        {
            return (btn1 != other.btn1) || (btn2 != other.btn2);
        }

        /** \brief Checks if the pad has changed state since the last packet. */
        constexpr bool HasPadChanged(const State& other) const noexcept
        {
            return (up != other.up) || (right != other.right) || (down != other.down) || (left != other.left);
        }
    };

    int GetMovementSpeed() const noexcept;

    std::optional<double> m_timeNs{}; /**< std::nullopt means a new packet can be sent, otherwise increments until delay is reached. */

    GamepadSpeed m_speed{GamepadSpeed::N};
    static constexpr int ACCELERATION_DELAY = 8; /**< Number of packets it takes to reach max cursor speed. */
    int m_consecutivePackets{0}; /**< Between 0 and 8, controls the acceleration of the cursor. */
    constexpr void IncrementConsecutivePacket() noexcept { m_consecutivePackets = std::min(m_consecutivePackets + 1, ACCELERATION_DELAY); }

    std::mutex m_stateMutex{}; /**< Mutex to hold when accessing \ref m_state. */
    State m_state{}; /**< Holds the state between packets. Acquire \ref m_stateMutex first. */
    State m_lastState{}; /**< Holds the state to return when GetState() is called, after a packet has been sent. */
};

} // namespace Pointing

#endif // CDI_POINTING_HPP
