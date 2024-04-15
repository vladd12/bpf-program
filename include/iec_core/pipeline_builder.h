#pragma once

#include <iec_core/engines/base_engine.h>
#include <memory>
#include <thread>

namespace pipeline
{

template <typename BufferType, typename ExchangeType, typename EngineType, typename HandlerType> //
class PipelineBuilder
{
public:
    // Types
    using Buffer = BufferType;
    using Exchange = utils::rebind_t<ExchangeType, Buffer>;
    using Engine = utils::rebind_t<EngineType, Exchange>;
    using Handler = utils::rebind_t<HandlerType, Exchange>;

private:
    Engine engine;
    Handler handler;
    Exchange exchange;
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
