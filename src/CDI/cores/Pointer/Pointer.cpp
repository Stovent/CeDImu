#include "Pointer.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <print>
#include <utility>

Pointer::Pointer(CDI& cdi)
    : m_cdi{cdi}
    , m_maneuvering{}
{
}

void Pointer::Reset() noexcept
{
    // m_maneuvering.Reset();
}

/** \brief Increments the emulated time.
 * \param ns The time to advance in nanoseconds.
 * \return The vector number to trigger to the CPU if required, std::nullopt otherwise.
 */
std::optional<uint8_t> Pointer::IncrementTime(const double ns) noexcept
{
    const bool irq = m_maneuvering.IncrementTime(ns);
    if(irq)
        return IRQ_VECTOR;

    return std::nullopt;
}

/** \brief Reads the last packet of the pointing device and writes it at the given address.
 * \param addr The address where to write (must be an array of 4 bytes).
 * `addr[0]`: Pointer state in msb.
 * `addr[1]`: Buttons state (bit 0 = button 1, bit 1 = button 2).
 * `addr[2]`: Pointer horizontal offset.
 * `addr[3]`: Pointer vertical offset.
 */
void Pointer::GetPacket(const uint32_t addr) noexcept
{
    std::array<uint8_t, 3> packet{};
    m_maneuvering.GetState(packet);

    uint8_t buttons = 0;
    if(bit<5>(packet[0]))
        buttons |= 0b01;
    if(bit<4>(packet[0]))
        buttons |= 0b10;

    const uint8_t horizontal = (packet[0] << 6) | packet[1];
    const uint8_t vertical = (packet[0] << 4 & 0xC0) | packet[2];

    m_cdi.SetByte(addr, 0x80, BUS_NORMAL); // Pointer state always active for maneuvering.
    m_cdi.SetByte(addr + 1, buttons, BUS_NORMAL);
    m_cdi.SetByte(addr + 2, horizontal, BUS_NORMAL);
    m_cdi.SetByte(addr + 3, vertical, BUS_NORMAL);
}
