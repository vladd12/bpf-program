#pragma once

#include <iec_core/engines/base_engine.h>
#include <memory>
#include <thread>

template <typename Engine, typename Handler, typename Exchange> //
class PipelineBuilder
{
    Engine engine;
    Handler handler;
    Exchange buffer;
    std::unique_ptr<std::thread> engineThread;
    std::unique_ptr<std::thread> handlerThread;

public:
    explicit PipelineBuilder() = default;

    void run(const engines::EngineSettings &settings)
    {
        if (engine.setup(settings))
        {
            ;
        }
    }
};
