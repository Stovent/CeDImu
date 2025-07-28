#ifndef CDI_CORES_CDDRIVE_CDDRIVE_HPP
#define CDI_CORES_CDDRIVE_CDDRIVE_HPP

#include "../../CDIDisc.hpp"

#include <optional>

class CDI;

/** \brief Emulated CD drive that controls the data reading delays and interrupts to the CPU. */
class CDDrive
{
public:
    static constexpr double SECTOR_DELTA = 13'333'333.0; /**< The delta between two sector read (1/75th second). */

    explicit CDDrive(CDI& cdi, CDIDisc disc = CDIDisc());

    constexpr CDIDisc& GetDisc() noexcept { return m_disc; }

    std::optional<uint8_t> IncrementTime(double ns) noexcept;

    void StartPlaying(uint32_t startLsn, uint32_t sectorCount, uint32_t writeAddress) noexcept;

private:
    CDI& m_cdi;
    CDIDisc m_disc;
    double m_timeNs{0.0};

    uint32_t m_discLsn{}; /**< The Logical Sector Number currently being read. */
    uint32_t m_sectorsLeft{0}; /**< The number of sectors left to play. When 0, means the disc is not playing. */
    uint32_t m_writeAddress{}; /**< The memory address where sector data will be written. */
};

#endif // CDI_CORES_CDDRIVE_CDDRIVE_HPP
