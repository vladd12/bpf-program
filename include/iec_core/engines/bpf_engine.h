#pragma once

#include <iec_core/engines/base_engine.h>
#include <iec_core/engines/bpf_exec.h>
#include <iec_core/utils/socket.h>
#include <iostream>

namespace engines
{

constexpr auto buffer_size = 8192;

template <typename ValueExchangeType = utils::placeholder_t> //
class BPFEngine final : public BaseRunnable<ValueExchangeType>
{
public:
    // Types
    using Exchange = ValueExchangeType;
    using Buffer = typename Exchange::buffer_t;
    using Socket = utils::Socket;

private:
    Buffer buffer;
    BPFExecutor executor;
    Socket socket;

public:
    explicit BPFEngine() : BaseRunnable<ValueExchangeType>(), executor("bpf/ethernet-parse.c"), socket()
    {
    }

    bool setup(const EngineSettings &settings)
    {
        executor.filterSourceCode(settings.iface.data(), settings.sourceMAC.data(), settings.svID.data());
        auto status = executor.load();
        if (status.ok())
        {
            int socket_fd = -1;
            status = executor.getDeviceSocket(socket_fd, "iec61850_filter", settings.iface.data());
            if (status.ok() && socket_fd >= 0)
            {
                socket.setHandle(socket_fd);
                if (!socket.setNonBlockingMode())
                    std::cout << "Couldn't set non-blocking read mode for socket!\n";
                return true;
            }
        }
        std::cout << status.msg() << '\n';
        return false;
    }

    void run() override
    {
        while (this->running)
        {
            socket.readTo(buffer);
            if (buffer.getFreeSize() < 800)
            {
                this->exchange->set(std::move(buffer));
                buffer.reset();
            }
        }
    }
};

} // namespace engines
