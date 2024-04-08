#pragma once

#include <iec_core/handlers/base_handler.h>
#include <iec_core/iec/iec_parser.h>
#include <iec_core/iec/validator.h>
#include <iec_core/utils/fast_file.h>

namespace handlers
{

template <typename Exchange> //
class NumberCrusher final : public BaseHandler
{
private:
    iec::IecParser parser;

public:
    explicit NumberCrusher() = default;

    void run() override
    {
        while (running)
        {
            ;
        }
    }
};

} // namespace handlers
