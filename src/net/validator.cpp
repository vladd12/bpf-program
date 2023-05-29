#include <net/validator.h>
#include <stdexcept>

namespace net
{

Validator::Validator() : value(0), state(State::Initial), strategy(Strategy::ThrowException), missedCount(0), capturedCount(0)
{
}

void Validator::validate(const ui16 newValue, const ui8 diff)
{
    if (state == State::Initial)
        state = State::Correct;
    else
    {
        auto max = (diff == 1) ? max80p : max256p;
        if (newValue == min && value == max)
            state = State::Correct;
        else if ((value + diff) == newValue)
            state = State::Correct;
        // missed packet
        else
        {
            state = State::Incorrect;
            if (strategy == Strategy::ThrowException)
                throw std::runtime_error("Packet missed");
            else if (strategy == Strategy::Statistics)
                missedCount += newValue - (value + diff);
        }

        if (strategy == Strategy::Statistics)
            capturedCount += diff;
    }
    value = newValue;
}

void Validator::setStrategy(Strategy newStrategy) noexcept
{
    strategy = newStrategy;
}

void Validator::update(const iec::SeqASDU &sequnce)
{
    const auto diff = sequnce.count;
    if (diff > 0 && sequnce.data != nullptr)
    {
        const auto newVal = sequnce.data[0].smpCnt;
        validate(newVal, diff);
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

}
