#ifndef CDI_CORES_MCD221_MCD221_HPP
#define CDI_CORES_MCD221_MCD221_HPP

class CDI;
#include "../../common/types.hpp"

#include <array>
#include <cstdint>

class MCD221
{
public:
    CDI& cdi;

    MCD221() = delete;
    MCD221(CDI& idc);

    void IncrementTime(const double ns);

    uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger);

    void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger);

private:
    std::array<uint16_t, 0x2600 / 2> registers;
};

#endif // CDI_CORES_MCD221_MCD221_HPP
