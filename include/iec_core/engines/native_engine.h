#pragma once

#include <iec_core/engines/base_engine.h>

namespace engines
{

template <typename ValueExchangeType = utils::placeholder_t> //
class NativeEngine : public BaseRunnable<ValueExchangeType>
{
private:
    int temp;

public:
    explicit NativeEngine() : BaseRunnable<ValueExchangeType>()
    {
    }

    bool setup(const EngineSettings &settings)
    {
        (void)(settings);
        temp = 1;
        return false;
    }

    void run() override
    {
        ;
    }
};

} // namespace engines
