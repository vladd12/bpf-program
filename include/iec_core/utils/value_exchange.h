#pragma once

#include <array>
#include <atomic>
#include <optional>

namespace utils
{

/// \brief Lock-free value-exchange data structure.
template <typename T> //
struct ValueExchange
{
private:
    std::array<std::optional<T>, 3> storage;
    std::optional<T> *set_buffer = &storage[0];
    std::atomic<std::optional<T> *> buffer = &storage[1];
    std::optional<T> *get_buffer = &storage[2];

public:
    explicit ValueExchange() = default;

    /// \brief Lock-free setting value.
    bool set(T &&value) noexcept
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
        return set(std::move(T { std::forward<Ts>(args)... }));
    }

    /// \brief Lock-free getting value.
    bool get(T &value) noexcept
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

} // namespace utils
