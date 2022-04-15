#ifndef CDI_CORES_MC68HC05_SCI_HPP
#define CDI_CORES_MC68HC05_SCI_HPP

#include <cstdint>
#include <functional>
#include <optional>

/** @class SCI
 * @brief SCI subsystem for the MC68HC05 microcontrollers.
 */
class SCI
{
public:
    enum RegistersFlags
    {
        // Control 2
        RE   = 0x04,
        TE   = 0x08,
        RIE  = 0x20,
        TCIE = 0x40,
        TIE  = 0x80,

        // Status
        OR   = 0x08,
        RDRF = 0x20,
        TC   = 0x40,
        TDRE = 0x80,
    };
    uint8_t controlRegister1;
    uint8_t controlRegister2;
    uint8_t baudRegister;

    SCI() = delete;
    explicit SCI(const std::function<void(uint16_t)> sendDataCallback);

    void Reset();
    [[nodiscard]] bool AdvanceCycles(size_t cycles);
    [[nodiscard]] bool ReceiveData(uint16_t data);

    [[nodiscard]] bool SetDataRegister(uint8_t data);
    uint8_t GetDataRegister();
    uint8_t GetStatusRegister();

private:
    uint8_t receivedData;
    uint8_t statusRegister;

    // First is the data to send, can be 8 or 9 bits. Second is the number of cycles before actually sending the data.
    std::optional<std::pair<uint16_t, uint64_t>> sciTransmit;
    uint8_t tdrBuffer; // Transmit Data Register buffer.
    std::function<void(uint16_t)> SendData;
    bool tdreTcRead; // To clear TDRE and TC bits of the status register.

    bool LoadTransmitter();
    size_t GetBaudRate() const;
};

#endif // CDI_CORES_MC68HC05_SCI_HPP
