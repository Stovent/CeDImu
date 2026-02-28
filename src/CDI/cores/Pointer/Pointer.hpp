#ifndef CDI_CORES_POINTER_POINTER_HPP
#define CDI_CORES_POINTER_POINTER_HPP

#include "../../Pointing.hpp"

#include <optional>

class CDI;

/** \brief Emulated pointer input chip that receives controller inputs and sends interrupts to the CPU. */
class Pointer
{
public:
    static constexpr uint8_t IRQ_VECTOR = 201; /**< The exception vector number used for the IRQ to the CPU. */

    explicit Pointer(CDI& cdi);

    void Reset() noexcept;

    std::optional<uint8_t> IncrementTime(double ns) noexcept;

    void SetUp(const bool pressed) noexcept { m_maneuvering.SetUp(pressed); }
    void SetRight(const bool pressed) noexcept { m_maneuvering.SetRight(pressed); }
    void SetDown(const bool pressed) noexcept { m_maneuvering.SetDown(pressed); }
    void SetLeft(const bool pressed) noexcept { m_maneuvering.SetLeft(pressed); }
    void SetButton1(const bool pressed) noexcept { m_maneuvering.SetButton1(pressed); }
    void SetButton2(const bool pressed) noexcept { m_maneuvering.SetButton2(pressed); }
    void SetButton12(const bool pressed) noexcept { m_maneuvering.SetButton12(pressed); }

    void GetPacket(uint32_t addr) noexcept;

private:
    CDI& m_cdi;

    // Pointing::RelativeCoordinate m_maneuvering;
    Pointing::Maneuvering m_maneuvering;
};

#endif // CDI_CORES_POINTER_POINTER_HPP
