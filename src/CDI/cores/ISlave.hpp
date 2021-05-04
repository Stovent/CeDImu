#ifndef ISLAVE_HPP
#define ISLAVE_HPP

#include "SCC68070/SCC68070.hpp"

#include <cstdint>

class ISlave
{
public:
    SCC68070& cpu;
    ISlave() = delete;
    explicit ISlave(SCC68070& scc68070) : cpu(scc68070) {}
    virtual ~ISlave() {}

    virtual uint8_t GetByte(const uint8_t addr) = 0;
    virtual void SetByte(const uint8_t addr, const uint8_t data) = 0;
};

#endif // ISLAVE_HPP
