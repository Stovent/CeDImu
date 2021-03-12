#ifndef HLE_IKAT_HPP
#define HLE_IKAT_HPP

#include "../../cores/ISlave.hpp"

namespace HLE
{

class IKAT : ISlave
{
public:
    virtual uint8_t GetByte(const uint8_t addr) override;
    virtual void SetByte(const uint8_t addr, const uint8_t data) override;
};

} // namespace HLE

#endif // HLE_IKAT_HPP
