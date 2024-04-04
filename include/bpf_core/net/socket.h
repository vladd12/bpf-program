#pragma once

#include <bpf_core/utils.h>
#include <sys/socket.h>

namespace net
{

/// \brief Class for wrapping the Linux sockets API.
class Socket
{
private:
    int sock_fd;
    timeval time;

public:
    explicit Socket();
    explicit Socket(int fd);

    /// \brief Set a new socket handle.
    /// \see getHandle
    void setHandle(int fd);

    /// \brief Returns a current socket handle.
    /// \see setHandle
    int getHandle();

    /// \brief Set a timeout interval to wait until the socket receives input data.
    /// \see isAvailable.
    void setWaitInterval(int sec = 0, int usec = 0) noexcept;

    /// \brief Checking that the socket receive input data.
    /// \see setWaitInterval.
    bool isAvailable() noexcept;

    /// \brief Closing the socket descriptor provided by the Linux API.
    void closeSock();

    /// \brief Blocking data reading from the socket to the buffer.
    /// \details The thread waits until the socket receive input data,
    /// and then begins reading data to the buffer.
    void blockRead(util::Buffer &buf);

    /// \brief Non-blocking data reading from the socket to the buffer.
    /// \details Checks that the socket has received input data.
    /// If the data is available, then they are read to the buffer.
    /// Otherwise, the CPU time of the thread is given to another process.
    /// \see isAvailable.
    void nonBlockRead(util::Buffer &buf);
};

}
