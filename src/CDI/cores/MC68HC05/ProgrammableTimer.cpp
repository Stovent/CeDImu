#include "ProgrammableTimer.hpp"

/** \brief New programmable timer.
 * \param timerCallback The argument is true if output compare level is high, false for low.
 */
ProgrammableTimer::ProgrammableTimer(std::function<void(bool)> outputCompareCallback)
    : controlRegister(0)
    , counterHigh(0xFF)
    , counterLow(0xFC)
    , inputCaptureAHigh(0)
    , inputCaptureALow(0)
    , inputCaptureBHigh(0)
    , inputCaptureBLow(0)
    , outputCompareHigh(0)
    , outputCompareLow(0)
    , statusRegister(0)
    , tofAccessed(false)
    , ocfAccessed(false)
    , icafAccessed(false)
    , icbfAccessed(false)
    , tcapPinA(false)
    , tcapPinB(false)
    , inputCaptureAInhibited(false)
    , inputCaptureBInhibited(false)
    , OutputCompareCallback(outputCompareCallback)
    , outputCompareInhibited(false)
    , counterLowBuffer()
    , alternateCounterLowBuffer()
{
}

void ProgrammableTimer::Reset()
{
    controlRegister &= 0x06; // 00000UU0
    counterHigh = 0xFF;
    counterLow = 0xFC;
    statusRegister &= 0xE0; // UUU00000
    tofAccessed = false;
    ocfAccessed = false;
    icafAccessed = false;
    icbfAccessed = false;

    tcapPinA = false;
    tcapPinB = false;
    inputCaptureAInhibited = false;
    inputCaptureBInhibited = false;
    outputCompareInhibited = false;
    counterLowBuffer.reset();
    alternateCounterLowBuffer.reset();
}

/** \brief Advances the timer for the given number of timer cycles (internal frequency / 4).
 * \return true if an interrupt has to be triggered, false otherwise.
 */
bool ProgrammableTimer::AdvanceCycles(size_t cycles)
{
    uint32_t counter = (uint32_t)counterHigh << 8 |counterLow;
    counter += cycles;
    counterHigh = counter >> 8;
    counterLow = counter;

    bool interrupt = false;
    if(counter > UINT16_MAX)
    {
        statusRegister |= TOF;
        if(controlRegister & TOIE)
            interrupt = true;
    }

    if(counterHigh == outputCompareHigh && counterLow == outputCompareLow && !outputCompareInhibited)
    {
        statusRegister |= OCF;
        OutputCompareCallback(controlRegister & OLVL);
        if(controlRegister & OCIE)
            interrupt = true;
    }

    return interrupt;
}

bool ProgrammableTimer::SetTCAPA(bool high)
{
    const bool oldHigh = tcapPinA;
    tcapPinA = high;
    if(inputCaptureAInhibited)
        return false;

    if((!oldHigh && high && controlRegister & IEDGA) || // Rising edge.
       (oldHigh && !high && !(controlRegister & IEDGA))) // Falling edge.
    {
        uint16_t counter = (uint16_t)counterHigh << 8 | counterLow;
        counter++; // 7.2.3.1
        inputCaptureAHigh = counter >> 8;
        inputCaptureALow = counter;

        statusRegister |= ICAF;
        if(controlRegister & ICAIE)
            return true;
    }

    return false;
}

bool ProgrammableTimer::SetTCAPB(bool high)
{
    const bool oldHigh = tcapPinB;
    tcapPinB = high;
    if(inputCaptureBInhibited)
        return false;

    if((!oldHigh && high && controlRegister & IEDGB) || // Rising edge.
       (oldHigh && !high && !(controlRegister & IEDGB))) // Falling edge.
    {
        uint16_t counter = (uint16_t)counterHigh << 8 | counterLow;
        counter++; // 7.2.3.2
        inputCaptureBHigh = counter >> 8;
        inputCaptureBLow = counter;

        statusRegister |= ICBF;
        if(controlRegister & ICBIE)
            return true;
    }

    return false;
}

uint8_t ProgrammableTimer::GetCounterHighRegister()
{
    if(!counterLowBuffer)
        counterLowBuffer = counterLow;
    return counterHigh;
}

uint8_t ProgrammableTimer::GetCounterLowRegister()
{
    if(tofAccessed)
        statusRegister &= ~TOF;

    if(counterLowBuffer)
    {
        const uint8_t val = counterLowBuffer.value();
        counterLowBuffer.reset();
        return val;
    }

    return counterLow; // TODO: clear TOF;
}

uint8_t ProgrammableTimer::GetAlternateCounterHighRegister()
{
    if(!alternateCounterLowBuffer)
        alternateCounterLowBuffer = counterLow;
    return counterHigh;
}

uint8_t ProgrammableTimer::GetAlternateCounterLowRegister()
{
    if(alternateCounterLowBuffer)
    {
        const uint8_t val = alternateCounterLowBuffer.value();
        alternateCounterLowBuffer.reset();
        return val;
    }

    return counterLow;
}

uint8_t ProgrammableTimer::GetInputCaptureAHighRegister()
{
    inputCaptureAInhibited = true;
    return inputCaptureAHigh;
}

uint8_t ProgrammableTimer::GetInputCaptureALowRegister()
{
    inputCaptureAInhibited = false;
    if(icafAccessed)
        statusRegister &= ~ICAF;
    return inputCaptureALow;
}

uint8_t ProgrammableTimer::GetInputCaptureBHighRegister()
{
    inputCaptureBInhibited = true;
    return inputCaptureBHigh;
}

uint8_t ProgrammableTimer::GetInputCaptureBLowRegister()
{
    inputCaptureBInhibited = false;
    if(icbfAccessed)
        statusRegister &= ~ICBF;
    return inputCaptureBLow;
}

uint8_t ProgrammableTimer::GetOutputCompareHighRegister()
{
    return outputCompareHigh;
}

uint8_t ProgrammableTimer::GetOutputCompareLowRegister()
{
    if(ocfAccessed)
        statusRegister &= ~OCF;
    return outputCompareLow;
}

uint8_t ProgrammableTimer::GetStatusRegister()
{
    tofAccessed = true;
    ocfAccessed = true;
    icafAccessed = true;
    icbfAccessed = true;

    return statusRegister;
}

void ProgrammableTimer::SetOutputCompareHighRegister(uint8_t value)
{
    outputCompareInhibited = true;
    outputCompareHigh = value;
}

void ProgrammableTimer::SetOutputCompareLowRegister(uint8_t value)
{
    outputCompareInhibited = false;
    outputCompareLow = value;
}
