#pragma once

#include <sys/socket.h>
#include <utils.h>

namespace Net
{

class Socket
{
private:
    int sock_fd;
    timeval time;

public:
    explicit Socket() = delete;
    explicit Socket(int fd);

    void setWaitInterval(int sec = 0, int usec = 0) noexcept;
    bool isAvailable() noexcept;
    void closeSock();
    void blockRead(util::Buffer &buf);
    void nonBlockRead(util::Buffer &buf);
};

}
