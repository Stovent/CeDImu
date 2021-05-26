#ifndef MANEUVERING_HPP
#define MANEUVERING_HPP

#include "PointingDevice.hpp"

enum class GamepadSpeed
{
    N,
    I,
    II,
};

class ManeuveringDevice : public PointingDevice
{
    uint64_t timer;
    uint32_t cursorSpeed;

public:
    ManeuveringDevice() = delete;
    explicit ManeuveringDevice(ISlave& slv);

    void SetCursorSpeed(const GamepadSpeed speed);

    virtual void IncrementTime(const size_t ns) override;
};

#endif // MANEUVERING_HPP
