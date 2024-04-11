#pragma once

#include <iec_core/engines/bpf_engine.h>
#include <iec_core/handlers/number_crusher.h>
#include <memory>
#include <thread>

namespace pipeline
{

// template <typename Engine, typename Handler, typename Exchange> //
class PipelineBuilder
{
    engines::BPFEngine engine;
    handlers::NumberCrusher handler;
    engines::BPFEngine::Exchange exchange;
    std::thread engineThread;
    std::thread handlerThread;

public:
    explicit PipelineBuilder() = default;

    void run(const engines::EngineSettings &settings)
    {
        if (engine.setup(settings))
        {
            engine.setExchange(exchange);
            handler.setExchange(exchange);
            engineThread = std::thread([this] { engine.run(); });
            handlerThread = std::thread([this] { handler.run(); });
        }
    }

    void wait()
    {
        if (engineThread.joinable())
            engineThread.join();
        if (handlerThread.joinable())
            handlerThread.join();
    }

    void stop()
    {
        engine.stop();
        handler.stop();
        exchange.reset();
        wait();
    }
};

} // namespace pipeline
