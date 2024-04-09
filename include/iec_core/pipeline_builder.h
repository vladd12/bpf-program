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
    std::unique_ptr<std::thread> engineThread;
    std::unique_ptr<std::thread> handlerThread;

public:
    explicit PipelineBuilder() = default;

    void run(const engines::EngineSettings &settings)
    {
        if (engine.setup(settings))
        {
            engine.setExchange(exchange);
            handler.setExchange(exchange);
            engineThread.reset(new std::thread([this] { engine.run(); }));
            handlerThread.reset(new std::thread([this] { handler.run(); }));
        }
    }

    void wait()
    {
        engineThread->join();
        handlerThread->join();
    }

    void stop()
    {
        engine.stop();
        handler.stop();
        exchange.set(engines::BPFEngine::Buffer {});
        wait();
    }
};

} // namespace pipeline
