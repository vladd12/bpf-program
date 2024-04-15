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

} // namespace engines

/// \brief Base class for runnable objects.
template <typename ValueExchangeType> //
class BaseRunnable
{
public:
    using Exchange = ValueExchangeType;
    using Buffer = typename Exchange::buffer_t;

protected:
    bool running;
    Exchange *exchange;

public:
    explicit BaseRunnable() noexcept : running(true)
    {
    }

    /// \brief Starts the execution flow for the runnable object.
    virtual void run() = 0;

    /// \brief Stops the running thread.
    void stop() noexcept
    {
        running = false;
    }

    /// \brief Setup value-exchange data structure for data sharing between threads.
    void setExchange(Exchange &exchange_) noexcept
    {
        exchange = &exchange_;
    }
};
