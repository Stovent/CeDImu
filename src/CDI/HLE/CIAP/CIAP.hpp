#ifndef CDI_HLE_CIAP_CIAP_HPP
#define CDI_HLE_CIAP_CIAP_HPP

class CDI;
#include "../../common/Cycles.hpp"
#include "../../common/types.hpp"

#include <array>
#include <cstdint>

class CIAP
{
public:
    CDI& cdi;

    CIAP() = delete;
    explicit CIAP(CDI& idc);

    void IncrementTime(const Cycles& c);

    uint16_t GetWord(const uint32_t addr, const uint8_t flags = Log | Trigger);

    void SetWord(const uint32_t addr, const uint16_t data, const uint8_t flags = Log | Trigger);

private:
    std::array<uint16_t, 0x2600 / 2> registers;
};

#endif // CDI_HLE_CIAP_CIAP_HPP
