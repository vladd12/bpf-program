#pragma once

#include <string_view>

namespace engines
{

struct EngineSettings
{
    std::string_view iface;     // enp0s8 - VM, eth0 - hardware
    std::string_view sourceMAC; // 0x0cefaf3042cc - 80p, 0x0cefaf3042cd - 256p
    std::string_view svID;      // ENS80pointMU01 - 80p, ENS256MUnn01 - 256p
};

class BaseEngine
{
protected:
    bool running = true;

public:
    explicit BaseEngine() = default;

    virtual bool setup(const EngineSettings &settings) = 0;
    virtual void run() = 0;

    void stop() noexcept
    {
        running = false;
    }
};

} // namespace engines
