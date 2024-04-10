#pragma once

#include <iec_core/handlers/base_handler.h>
#include <iec_core/iec/iec_parser.h>
#include <iec_core/iec/validator.h>
#include <iec_core/utils/buffer.h>
#include <iec_core/utils/fast_file.h>
#include <iec_core/utils/value_exchange.h>

namespace handlers
{

constexpr auto buffer_size = 8192;

class NumberCrusher final : public BaseHandler
{
public:
    // Types
    using Buffer = utils::StaticBuffer<buffer_size>;
    using Exchange = utils::ValueExchangeBlocking<Buffer>;
    using Parser = iec::IecParser;
    using Validator = iec::Validator;

private:
    Buffer buffer;
    Parser parser;
    Validator validator;
    Exchange *exchange;

public:
    explicit NumberCrusher() = default;

    void run() override
    {
        while (running)
        {
            exchange->get(buffer);
            auto sequence { parser.parse(buffer) };
            validator.update(sequence);
            [[maybe_unused]] auto points { parser.convert(sequence) };
        }
    }

    void setExchange(Exchange &exchange_) noexcept
    {
        exchange = &exchange_;
    }
};

} // namespace handlers
