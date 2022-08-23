#include "SCI.hpp"
#include "MC68HC05.hpp"

/** @brief New SCI subsystem.
 * @param sendDataCallback The callback called when the transmitter will send data.
 */
SCI::SCI(const std::function<void(uint16_t)> sendDataCallback)
    : controlRegister1(0)
    , controlRegister2(0)
    , baudRegister(0)
    , receivedData(0)
    , statusRegister(0xC0)
    , sciTransmit()
    , tdrBuffer(0)
    , SendData(sendDataCallback)
    , tdreTcRead(false)
{
}

void SCI::Reset()
{
    controlRegister1 &= 0xC0; // UU000000
    controlRegister2 = 0;
    baudRegister &= 0x07; // -000-UUU
    receivedData = 0;
    statusRegister = 0xC0;

    tdrBuffer = 0;
    tdreTcRead = false;
}

/** @brief Emulates the SCI subsystem for the given number of cycles.
 * @return true if an interrupt has to be triggered, false otherwise.
 */
bool SCI::AdvanceCycles(size_t cycles)
{
    if(!sciTransmit)
        return false;

    if(sciTransmit.value().second > cycles)
    {
        sciTransmit.value().second -= cycles;
        return false;
    }

    SendData(sciTransmit.value().first);
    sciTransmit.reset();

    bool interrupt = false;

    if(statusRegister & TDRE) // 9.2 TC flag is set (provided no pending data [...] is to be sent).
    {
        statusRegister |= TC;
        if(controlRegister2 & TCIE)
            interrupt = true;
    }

    interrupt = interrupt || LoadTransmitter();

    return interrupt;
}

/** @brief Sets the data received by the SCI receiver.
 * @return true if an interrupt has to be triggered, false otherwise.
 */
bool SCI::ReceiveData(uint16_t data)
{
    if(controlRegister2 & RE) // Receiver enabled.
    {
        if(!(statusRegister & RDRF))
        {
            receivedData = data;
            controlRegister1 &= 0x58; // Clear R8 bit.
            if(controlRegister1 & 0x10) // 9 bits enable.
                controlRegister1 |= data >> 1 & 0x0080;

            statusRegister |= RDRF;
        }
        else
            statusRegister |= OR;

        if(controlRegister2 & RIE)
            return true;
    }

    return false;
}

/** @brief Sets the data register to the given byte.
 * @return true if an interrupt has to be triggered, false otherwise.
 */
bool SCI::SetDataRegister(uint8_t data)
{
    if(controlRegister2 & TE) // Transmitter enable.
    {
        tdrBuffer = data;
        if(tdreTcRead)
        {
            statusRegister &= ~(TDRE | TC);
            tdreTcRead = false;
        }
        return LoadTransmitter();
    }

    return false;
}

/** @brief Returns the data in SCDR.
 */
uint8_t SCI::GetDataRegister()
{
    statusRegister &= TDRE | TC; // Clear RDRF and OR.
    return receivedData;
}

uint8_t SCI::GetStatusRegister()
{
    tdreTcRead = true;
    return statusRegister;
}

/** @brief Transfers the tdrBuffer to the sciTransmit.
 * @return true if an interrupt has to be triggered, false otherwise.
 */
bool SCI::LoadTransmitter()
{
    if(statusRegister & TDRE || sciTransmit)
        return false;

    uint16_t data = tdrBuffer;
    if((controlRegister1 & 0x50) == 0x50) // 9 bits enable and 9th bit set to 1.
        data |= 0x100;

    const size_t baudRate = GetBaudRate();
    const double cyclesPerBit = static_cast<double>(MC68HC05::INTERNAL_BUS_FREQUENCY) / static_cast<double>(baudRate);
    const uint64_t bits = controlRegister1 & 0x10 ? 11 : 10;
    sciTransmit = std::make_pair(data, static_cast<uint64_t>(static_cast<double>(bits) * cyclesPerBit));

    statusRegister |= TDRE;
    if(controlRegister2 & TIE)
        return true;
    return false;
}

size_t SCI::GetBaudRate() const
{
    constexpr size_t PRESCALER[4] = {1, 3, 4, 13};
    const size_t scp = baudRegister >> 4 & 3;
    const size_t sciPrescaler = PRESCALER[scp];

    const size_t scr = baudRegister & 7;
    const size_t sciRate = 1 << scr;

    return MC68HC05::INTERNAL_BUS_FREQUENCY / sciPrescaler / sciRate / 16; // Figure 3.24 or 9-2.
}
