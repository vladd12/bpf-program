#pragma once

#include <exception>
#include <fast_file.h>
#include <iec_parser.h>
#include <iostream>
#include <net/sock.h>

namespace net
{

enum class State : ui8
{
    Initial = 0,
    Correct,
    Incorrect
};

class Validator
{
private:
    static constexpr std::uint16_t min = 0;
    static constexpr std::uint16_t max80p = 3999;
    static constexpr std::uint16_t max256p = 5000;

    std::uint16_t value;
    State state;

    void update(const std::uint16_t newValue, const std::uint8_t diff) noexcept
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

    void validate()
    {
        if (state == State::Incorrect)
            throw std::runtime_error("Packet missed");
    }

public:
    explicit Validator() : value(0), state(State::Initial)
    {
    }

    void update(const iec::SeqASDU &sequnce)
    {
        const auto diff = sequnce.count;
        if (diff > 0 && sequnce.data != nullptr)
        {
            const auto newVal = sequnce.data[0].smpCnt;
            update(newVal, diff);
            validate();
        }
        else
        {
            throw std::runtime_error("Incorrect sequence of ASDU received");
        }
    }
};

class NetDriver
{
private:
    util::Buffer buf;
    Socket sock;
    iec::IecParser parser;
    FastFile output;
    Validator validator;

    void allocBuffer()
    {
        buf.data = new ui8[buf.allocSize];
    }

public:
    explicit NetDriver() = delete;
    explicit NetDriver(const Socket &socket, const std::string &filename)
        : buf { nullptr, util::BUFFER_SIZE, 0 }, sock(socket), output(filename)
    {
        allocBuffer();
    }

    void run()
    {
        while (true)
        {
            sock.nonBlockRead(buf);
            if (parser.update(buf.data, buf.readSize))
            {
                auto sequence = parser.parse();
                validator.update(sequence);
            }
        }
    }
};

}
