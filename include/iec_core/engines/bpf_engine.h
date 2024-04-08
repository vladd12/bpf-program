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
private:
    utils::StaticBuffer<buffer_size> buffer;
    BPFExecutor executor;
    utils::Socket socket;

public:
    explicit BPFEngine();

    bool setup(const EngineSettings &settings) override;
    void run() override;
};

} // namespace engines
