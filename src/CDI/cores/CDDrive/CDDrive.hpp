#ifndef CDI_CORES_CDDRIVE_CDDRIVE_HPP
#define CDI_CORES_CDDRIVE_CDDRIVE_HPP

#include "../../CDIDisc.hpp"

#include <optional>

class CDI;

/** \brief Emulated CD drive that controls the data reading delays and interrupts to the CPU. */
class CDDrive
{
public:
    static constexpr uint8_t EXCEPTION_VECTOR = 200; /**< The exception vector number used for the IRQ to the CPU. */
    static constexpr double SECTOR_DELTA = 13'333'333.0; /**< The delta between two sector read (1/75th second). */

    explicit CDDrive(CDI& cdi, CDIDisc disc = CDIDisc());

    void Reset() noexcept;

    constexpr CDIDisc& GetDisc() noexcept { return m_disc; }

    std::optional<uint8_t> IncrementTime(double ns) noexcept;

    void StartPlaying(uint32_t startLsn, uint32_t sectorCount, uint8_t file, uint32_t channelMask) noexcept;
    void CopyLastSectorToMemory(uint32_t addr, uint32_t size) const noexcept;
    uint32_t GetLastSectorSubheader() const noexcept;

private:
    CDI& m_cdi;
    double m_timeNs{0.0};

    CDIDisc m_disc;
    CDISector m_lastSector{}; /**< The last sector read. */

    uint32_t m_discLsn{}; /**< The Logical Sector Number currently being read. */
    uint32_t m_sectorsLeft{0}; /**< The number of sectors left to play. When 0, means the disc is not playing. */
    uint8_t m_fileNumber{}; /**< The file number the sectors must be part of. */
    uint32_t m_channelMask{}; /**< The channel mask of the allowed sector channel number. */
};

#endif // CDI_CORES_CDDRIVE_CDDRIVE_HPP
