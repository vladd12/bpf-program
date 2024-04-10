#include "iec_core/iec/validator.h"

#include <stdexcept>

namespace iec
{

Validator::Validator() noexcept
    : capturedSmpSync(0), state(State::Initial), strategy(Strategy::ThrowException), missedCount(0), capturedCount(0)
{
}

void Validator::validate(const u16 smpSync)
{
    if (state == State::Initial)
        state = State::Correct;
    else
    {
        if (smpSync == min && (capturedSmpSync == max80p || capturedSmpSync == max256p))
            state = State::Correct;
        else if ((smpSync - 1) == capturedSmpSync)
            state = State::Correct;
        // missed one packet or more
        else
        {
            printf("Prev: %04X\nCurrent: %04X\nWait: %04X\n", capturedSmpSync, smpSync, capturedSmpSync + 1);
            state = State::Incorrect;
            if (strategy == Strategy::ThrowException)
                throw std::runtime_error("Packet missed");
            else if (strategy == Strategy::Statistics)
            {
                auto missed = smpSync - (capturedSmpSync + 1);
                printf("Missed: %d\n", missed);
                missedCount += missed;
            }
        }
    }
    if (strategy == Strategy::Statistics)
        ++capturedCount;
    capturedSmpSync = smpSync;
}

void Validator::setStrategy(Strategy newStrategy) noexcept
{
    strategy = newStrategy;
}

void Validator::update(const std::vector<ASDU> &sequnce)
{
    if (!sequnce.empty())
    {
        for (const auto &asdu : sequnce)
        {
            const auto newVal = asdu.smpCnt;
            // printf("Value: %04X\n", newVal);
            validate(newVal);
        }
    }
    else
        throw std::runtime_error("Incorrect sequence of ASDU received");
}

void Validator::reset() noexcept
{
    state = State::Initial;
    missedCount = 0;
    capturedCount = 0;
}

} // namespace iec
