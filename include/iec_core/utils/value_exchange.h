#pragma once

#include <array>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>

namespace utils
{

/// \brief Lock-free value-exchange data structure.
template <typename T> //
struct ValueExchange
{
public:
    using buffer_t = T; ///< Buffer data type.

private:
    std::array<std::optional<buffer_t>, 3> storage;
    std::optional<buffer_t> *set_buffer = &storage[0];
    std::atomic<std::optional<buffer_t> *> buffer = &storage[1];
    std::optional<buffer_t> *get_buffer = &storage[2];

public:
    explicit ValueExchange() = default;

    /// \brief Lock-free setting value.
    bool set(buffer_t &&value) noexcept
    {
        if (!set_buffer->has_value())
        {
            set_buffer->emplace(std::move(value));
            set_buffer = buffer.exchange(set_buffer);
            return true;
        }
        else
            return false;
    }

    /// \brief Lock-free setting value.
    template <typename... Ts> //
    bool set(Ts &&...args) noexcept
    {
        return set(std::move(buffer_t { std::forward<Ts>(args)... }));
    }

    /// \brief Lock-free getting value.
    bool get(buffer_t &value) noexcept
    {
        get_buffer = buffer.exchange(get_buffer);
        if (get_buffer->has_value())
        {
            value = std::move(get_buffer->value());
            get_buffer->reset();
            return true;
        }
        else
            return false;
    }
};

/// \brief Value-exchange data structure with mutex blocking.
template <typename T> //
struct ValueExchangeBlocking
{
public:
    using buffer_t = T; ///< Buffer data type.

private:
    buffer_t storage;
    std::mutex accessor;
    std::condition_variable waiter;
    bool isFilled;

public:
    explicit ValueExchangeBlocking() : isFilled(false)
    {
    }

    void get(buffer_t &value)
    {
        // wait if not filled yet
        if (!isFilled)
        {
            std::unique_lock<std::mutex> locker { accessor };
            waiter.wait(locker, [this] { return isFilled; });
        }
        // get value from storage and notify writer's thread
        std::lock_guard<std::mutex> locker { accessor };
        value = std::move(storage);
        isFilled = false;
        waiter.notify_one();
    }

    void set(buffer_t &&value)
    {
        // wait if already filled
        if (isFilled)
        {
            std::unique_lock<std::mutex> locker { accessor };
            waiter.wait(locker, [this] { return !isFilled; });
        }
        // set new value to storage and notify reader's thread
        std::lock_guard<std::mutex> locker { accessor };
        storage = std::move(value);
        isFilled = true;
        waiter.notify_one();
    }

    void reset()
    {
        isFilled = true;
        waiter.notify_one(); // notify reader's thread
        isFilled = false;
        waiter.notify_one(); // notify writer's thread
    }
};

} // namespace utils
