#pragma once

namespace handlers
{

class BaseHandler
{
protected:
    bool running = true;

public:
    explicit BaseHandler() = default;

    virtual void run() = 0;
    void stop() noexcept
    {
        running = false;
    }
};

} // namespace handlers
