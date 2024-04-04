#include "bpf_core/iec/validator.h"

#include <stdexcept>

namespace iec
{

Validator::Validator() noexcept
    : value(0), state(State::Initial), strategy(Strategy::ThrowException), missedCount(0), capturedCount(0)
{
}

void Validator::validate(const u16 svID, const u8 count)
{
    if (state == State::Initial)
        state = State::Correct;
    else
    {
        auto max = (count == 1) ? max80p : max256p;
        if (svID == min && value == max)
            state = State::Correct;
        else if ((svID - count) == value)
            state = State::Correct;
        // missed packet
        else
        {
            printf("Prev: %04X\nCurrent: %04X\n", value, svID);
            printf("Wait: %04X\n", value + count);
            state = State::Incorrect;
            if (strategy == Strategy::ThrowException)
                throw std::runtime_error("Packet missed");
            else if (strategy == Strategy::Statistics)
            {
                auto missed = svID - (value + count);
                printf("Missed: %d\n", missed);
                missedCount += missed;
            }
        }
    }
    if (strategy == Strategy::Statistics)
        capturedCount += count;
    value = svID;
}

void Validator::setStrategy(Strategy newStrategy) noexcept
{
    strategy = newStrategy;
}

void Validator::update(const iec::SeqASDU &sequnce)
{
    const auto count = sequnce.count;
    if (count > 0 && sequnce.data != nullptr)
    {
        const auto newVal = sequnce.data[0].smpCnt;
        // printf("Value: %04X\n", newVal);
        validate(newVal, count);
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
