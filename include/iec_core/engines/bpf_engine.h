#pragma once

#include <iec_core/engines/base_engine.h>
#include <iec_core/engines/bpf_exec.h>
#include <iec_core/utils/socket.h>
#include <iec_core/utils/value_exchange.h>

namespace engines
{

constexpr auto buffer_size = 8192;

class BPFEngine final : public BaseEngine
{
public:
    // Types
    using Buffer = utils::StaticBuffer<buffer_size>;
    using Socket = utils::Socket;
    using Exchange = utils::ValueExchangeBlocking<Buffer>;

private:
    Buffer buffer;
    BPFExecutor executor;
    Socket socket;
    Exchange *exchange;

public:
    explicit BPFEngine();

    bool setup(const EngineSettings &settings) override;
    void run() override;

    void setExchange(Exchange &exchange_) noexcept
    {
        exchange = &exchange_;
    }
};

} // namespace engines
