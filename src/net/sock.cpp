#include <net/sock.h>
#include <thread>

// Linux
#include <net/ethernet.h>
#include <unistd.h>

namespace net
{

Socket::Socket(int fd) : sock_fd(fd), time { 0, 1 }
{
}

void Socket::setWaitInterval(int sec, int usec) noexcept
{
    time.tv_sec = sec;
    time.tv_usec = usec;
}

bool Socket::isAvailable() noexcept
{
    fd_set fd_in;
    FD_ZERO(&fd_in);
    FD_SET(sock_fd, &fd_in);

    auto ret = select(FD_SETSIZE + 1, &fd_in, nullptr, nullptr, &time);
    // error
    if (ret == -1)
    {
        fprintf(stderr, "Error checking socket aviability");
        return false;
    }
    // timeout
    else if (ret == 0)
        return false;
    // ok
    else
        return true;
}

void Socket::closeSock()
{
    close(sock_fd);
}

void Socket::blockRead(util::Buffer &buf)
{
    buf.readSize = recvfrom(sock_fd, buf.data, buf.allocSize, 0, nullptr, nullptr);
}

void Socket::nonBlockRead(util::Buffer &buf)
{
    while (true)
    {
        if (isAvailable())
            buf.readSize = recvfrom(sock_fd, buf.data, buf.allocSize, 0, nullptr, nullptr);
        else
            std::this_thread::yield();
    }
}

}
