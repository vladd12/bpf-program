#pragma once

#include <iec_core/algo/processing.h>
#include <iec_core/engines/base_engine.h>
#include <iec_core/iec/validator.h>
#include <iec_core/utils/buffer.h>
#include <iec_core/utils/value_exchange.h>

namespace handlers
{

template <typename ValueExchangeType> //
class NumberCrusher final : public BaseRunnable<ValueExchangeType>
{
public:
    // Types
    using Exchange = ValueExchangeType;
    using Buffer = typename Exchange::buffer_t;
    using Parser = iec::IecParser;
    using Validator = iec::Validator;

private:
    Buffer buffer;
    Parser parser;
    Validator validator;
    std::vector<iec::Point> points;

public:
    explicit NumberCrusher() = default;

    void run() override
    {
        while (this->running)
        {
            this->exchange->get(buffer);
            auto sequence { parser.parse(buffer) };
            validator.update(sequence);
            parser.convert(sequence, points);
            if (points.size() > 512)
            {
                algo::findMinMax(points);
                points.clear();
            }
        }
    }
};

} // namespace handlers
