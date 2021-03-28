#ifndef HLE_IKAT_HPP
#define HLE_IKAT_HPP

#include "../../cores/ISlave.hpp"

namespace HLE
{

enum IKATRegisters
{
    PAWR = 0,
    PBWR,
    PCWR,
    PDWR,
    PARD,
    PBRD,
    PCRD,
    PDRD,
    PASR,
    PBSR,
    PCSR,
    PDSR,
    ISR,
    ICR,
    YCR,
};

class IKAT : public ISlave
{
    int index;
    int commandSize;
    const uint8_t* command;

public:
    IKAT();

    virtual uint8_t GetByte(const uint8_t addr) override;
    virtual void SetByte(const uint8_t addr, const uint8_t data) override;
};

} // namespace HLE

#endif // HLE_IKAT_HPP
