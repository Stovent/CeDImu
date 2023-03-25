#include "CoreTimer.hpp"

CoreTimer::CoreTimer()
    : controlStatusRegister(0b0000'0011)
    , counterRegister(0)
    , rtiCounter(0)
    , copCounter(0)
{
}

void CoreTimer::Reset()
{
    controlStatusRegister = 0x03;
    counterRegister = 0;
    rtiCounter = 0;
    copCounter = 0;
}

/** \brief Advances the timer for the given number of timer cycles (internal frequency / 4).
 * \return A pair indicating if an interrupt (.first) or a reset (.second) should happen.
 */
std::pair<bool, bool> CoreTimer::AdvanceCycles(size_t cycles)
{
    uint16_t counter = counterRegister;
    counter += cycles;
    counterRegister = counter;

    bool interrupt = false;
    bool reset = false;
    if(counter > UINT8_MAX)
    {
        controlStatusRegister |= CTOF;
        if(controlStatusRegister & CTOFE)
            interrupt = true;

        rtiCounter++;
        const uint8_t rtiShift = 3 + (controlStatusRegister & 3);
        if(rtiCounter >> rtiShift)
        {
            rtiCounter = 0; // Very unlikely to be cleared when it shouldn't.
            controlStatusRegister |= RTIF;
            if(controlStatusRegister & RTIE)
                interrupt = true;

            copCounter++;
            if(copCounter >> 3) // RTI counter divided by 8.
            {
                copCounter = 0; // Even more unlikely.
                reset = true;
            }
        }
    }

    return {interrupt, reset};
}

void CoreTimer::ClearCOP()
{
    copCounter = 0;
}

uint8_t CoreTimer::GetCounterRegister()
{
    return counterRegister;
}

uint8_t CoreTimer::GetControlStatusRegister()
{
    return controlStatusRegister;
}

void CoreTimer::SetControlStatusRegister(uint8_t value)
{
    controlStatusRegister &= 0xC0;
    controlStatusRegister |= value & 0x33;

    if(!(value & CTOF))
        controlStatusRegister &= ~CTOF;
    if(!(value & RTIF))
        controlStatusRegister &= ~RTIF;
}
