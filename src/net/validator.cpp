#include <net/validator.h>
#include <stdexcept>

namespace net
{

Validator::Validator() : value(0), state(State::Initial)
{
}

void Validator::update(const ui16 newValue, const ui8 diff)
{
    if (state == State::Initial)
    {
        value = newValue;
        state = State::Correct;
    }
    else
    {
        auto max = (diff == 1) ? max80p : max256p;
        if (newValue == min && value == max)
        {
            value = newValue;
            state = State::Correct;
        }
        else if ((value + diff) == newValue)
        {
            value = newValue;
            state = State::Correct;
        }
        else
        {
            value = newValue;
            state = State::Incorrect;
        }
    }
}

void Validator::validate()
{
    if (state == State::Incorrect)
        throw std::runtime_error("Packet missed");
}

void Validator::update(const iec::SeqASDU &sequnce)
{
    const auto diff = sequnce.count;
    if (diff > 0 && sequnce.data != nullptr)
    {
        const auto newVal = sequnce.data[0].smpCnt;
        update(newVal, diff);
        validate();
    }
    else
        throw std::runtime_error("Incorrect sequence of ASDU received");
}

}
