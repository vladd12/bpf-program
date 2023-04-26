#pragma once

#include <cstdint>
#include <iostream>
#include <thread>

// Linux net API
#include <net/ethernet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

struct Buffer
{
    std::uint8_t *data;
    std::size_t size;
};

class Socket
{
private:
    int sock_fd;
    timeval time;

public:
    explicit Socket() = delete;
    explicit Socket(int fd) : sock_fd(fd), time { 0, 1 }
    {
    }

    void setWaitInterval(int sec = 0, int usec = 0) noexcept
    {
        time.tv_sec = sec;
        time.tv_usec = usec;
    }

    bool isAvailable() noexcept
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

    void closeSock()
    {
        close(sock_fd);
    }

    std::size_t blockingRead(Buffer &buf)
    {
        return recvfrom(sock_fd, buf.data, buf.size, 0, nullptr, nullptr);
    }

    std::size_t nonblockingRead(Buffer &buf)
    {
        while (true)
        {
            if (isAvailable())
                return recvfrom(sock_fd, buf.data, buf.size, 0, nullptr, nullptr);
            else
                std::this_thread::yield();
        }
    }
};

class NetDriver
{
public:
    NetDriver();
};
