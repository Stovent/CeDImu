#include "CDDrive.hpp"
#include "../../CDI.hpp"

#include <utility>

CDDrive::CDDrive(CDI& cdi, CDIDisc disc)
    : m_cdi{cdi}
    , m_disc{std::move(disc)}
{
}

/** \brief Increments the emulated time.
 * \param ns The time to advance in nanoseconds.
 * \return The vector number to trigger to the CPU if required, std::nullopt otherwise.
 */
std::optional<uint8_t> CDDrive::IncrementTime(const double ns) noexcept
{
    if(m_sectorsLeft == 0 || !m_disc.IsOpen())
        return std::nullopt;

    m_timeNs += ns;
    if(m_timeNs < SECTOR_DELTA) // One sector every 1/75th second.
        return std::nullopt;
    m_timeNs -= SECTOR_DELTA;

    // Sector has been read.
    const std::optional<CDISector> sector = m_disc.GetSector(m_discLsn++);
    if(!sector)
        return std::nullopt; // TODO: panic

    for(uint8_t byte : sector->data)
        m_cdi.SetByte(m_writeAddress++, byte, BUS_NORMAL);
    --m_sectorsLeft;

    return {200}; // TODO: this needs to be taken from SoftCDI's driver source.
}

/** \brief Configures the drive to play the request sectors.
 * \param startLsn The starting Logical Sector Number (0 is 0:2:0).
 * \param sectorCount The number of sectors to play.
 * \param writeAddress The address where the data will be written to.
 */
void CDDrive::StartPlaying(const uint32_t startLsn, const uint32_t sectorCount, const uint32_t writeAddress) noexcept
{
    m_discLsn = startLsn;
    m_sectorsLeft = sectorCount == 0 ? 1 : sectorCount;
    m_writeAddress = writeAddress;
}
