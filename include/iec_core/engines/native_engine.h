#pragma once

#include <iec_core/engines/base_engine.h>

namespace engines
{

class NativeEngine : public BaseEngine
{
private:
    int temp;

public:
    explicit NativeEngine();

    bool setup(const EngineSettings &settings) override;
    void run() override;
};

} // namespace engines
