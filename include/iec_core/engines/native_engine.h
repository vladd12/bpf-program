#pragma once

#include <iec_core/engines/base_engine.h>

namespace engines
{

/// \brief The native network filter for input data.
template <typename ValueExchangeType> //
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
