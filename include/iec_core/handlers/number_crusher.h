#pragma once

#include <iec_core/handlers/base_handler.h>

namespace handlers
{

class NumberCrusher final : public BaseHandler
{
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
