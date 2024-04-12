#include "iec_core/utils/socket.h"

#include <fcntl.h>
#include <thread>
#include <unistd.h>

namespace utils
{

Socket::Socket() noexcept : sock_fd(-1)
{
}

Socket::Socket(int fd) noexcept : sock_fd(fd)
{
}

Socket::~Socket() noexcept
{
    closeSocket();
}

void Socket::closeSocket() noexcept
{
    if (sock_fd >= 0)
        ::close(sock_fd);
}

void Socket::setHandle(int fd) noexcept
{
    closeSocket();
    sock_fd = fd;
}

int Socket::getHandle() const noexcept
{
    return sock_fd;
}

bool Socket::setNonBlockingMode() noexcept
{
    auto flags = fcntl(sock_fd, F_GETFL);
    return (fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK) != -1);
}

}
