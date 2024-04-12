#pragma once

#include <iec_core/utils/buffer.h>
#include <sys/socket.h>

namespace utils
{

/// \brief Class for wrapping the Linux sockets API.
class Socket
{
private:
    int sock_fd;

    void closeSocket() noexcept;

public:
    explicit Socket() noexcept;
    explicit Socket(int fd) noexcept;
    ~Socket() noexcept;

    /// \brief Set a new socket handle.
    /// \see getHandle
    void setHandle(int fd) noexcept;

    /// \brief Returns a current socket handle.
    /// \see setHandle
    int getHandle() const noexcept;

    /// \brief Setup socket to non-blocking mode for I/O operations.
    bool setNonBlockingMode() noexcept;

    /// \brief Reading data from the socket to the static buffer.
    template <u64 size> //
    void readTo(StaticBuffer<size> &buffer)
    {
        auto result = recvfrom(sock_fd, buffer.getFree(), buffer.getFreeSize(), 0, nullptr, nullptr);
        if (result > 0)
            buffer.appendWritten(result);
    }
};

}
