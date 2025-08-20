#include "CDDrive.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <print>
#include <utility>

CDDrive::CDDrive(CDI& cdi, CDIDisc disc)
    : m_cdi{cdi}
    , m_disc{std::move(disc)}
{
}

void CDDrive::Reset() noexcept
{
    m_timeNs = 0.0;

    m_lastSector = {};

    m_discLsn = 0;
    m_sectorsLeft = 0;
    m_fileNumber = 0;
    m_channelMask = 0;
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
    m_lastSector = std::move(*sector);

    --m_sectorsLeft;

    // Only send an interrupt if the sector passes the filters.
    if(m_fileNumber == m_lastSector.subheader.fileNumber &&
       ((1 << m_lastSector.subheader.channelNumber) & m_channelMask) != 0)
        return {EXCEPTION_VECTOR};

    return std::nullopt; // Not a valid sector, don't IRQ.
}

/** \brief Configures the drive to play the request sectors.
 * \param startLsn The starting Logical Sector Number (0 is 0:2:0).
 * \param sectorCount The number of sectors to play.
 * \param file The file number the sectors has to be from.
 * \param channelMask The allowed channels as a bit string.
 */
void CDDrive::StartPlaying(uint32_t startLsn, uint32_t sectorCount, uint8_t file, uint32_t channelMask) noexcept
{
    m_discLsn = startLsn;
    m_sectorsLeft = sectorCount == 0 ? 1 : sectorCount;
    m_fileNumber = file;
    m_channelMask = channelMask;
}

/** \brief Copies the last read sector into emulated memory.
 * \param addr The address where the data will be written to.
 * \param size The size of the buffer.
 */
void CDDrive::CopyLastSectorToMemory(uint32_t addr, const uint32_t size) const noexcept
{
    if(size < m_lastSector.data.size())
        std::println("Warning: buffer size is {} < {}", size, m_lastSector.data.size());

    for(size_t i = 0; i < size && i < m_lastSector.data.size(); ++i)
        m_cdi.SetByte(addr++, m_lastSector.data[i], BUS_NORMAL);
}

/** \brief Returns the subheader as a single uint32_t integer.
 * From LSB to MSB we have file number, channel number, submode and coding information.
 */
uint32_t CDDrive::GetLastSectorSubheader() const noexcept
{
    return makeU32(m_lastSector.subheader.fileNumber, m_lastSector.subheader.channelNumber,
                   m_lastSector.subheader.submode, m_lastSector.subheader.codingInformation);
}
