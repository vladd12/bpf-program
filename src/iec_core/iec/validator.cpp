#include "iec_core/iec/validator.h"

#include <stdexcept>

namespace iec
{

Validator::Validator() noexcept
    : value(0), state(State::Initial), strategy(Strategy::ThrowException), missedCount(0), capturedCount(0)
{
}

void Validator::validate(const u16 svID)
{
    if (state == State::Initial)
        state = State::Correct;
    else
    {
        if (svID == min && (value == max80p || value == max256p))
            state = State::Correct;
        else if ((svID - 1) == value)
            state = State::Correct;
        // missed packet
        else
        {
            printf("Prev: %04X\nCurrent: %04X\nWait: %04X\n", value, svID, value + 1);
            state = State::Incorrect;
            if (strategy == Strategy::ThrowException)
                throw std::runtime_error("Packet missed");
            else if (strategy == Strategy::Statistics)
            {
                auto missed = svID - (value + 1);
                printf("Missed: %d\n", missed);
                missedCount += missed;
            }
        }
    }
    if (strategy == Strategy::Statistics)
        ++capturedCount;
    value = svID;
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
