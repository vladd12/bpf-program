#pragma once

#include <fast_file.h>
#include <iec_parser.h>
#include <iostream>
#include <net/sock.h>

namespace Net
{

class NetDriver
{
private:
    util::Buffer buf;
    Socket sock;
    iec::IecParser parser;
    FastFile output;

    void allocBuffer()
    {
        buf.data = new std::uint8_t[buf.allocSize];
    }

public:
    explicit NetDriver() = delete;
    explicit NetDriver(const Socket &socket, const std::string &filename)
        : buf { nullptr, util::BUFFER_SIZE, 0 }, sock(socket), output(filename)
    {
        allocBuffer();
    }

    void run()
    {
        while (true)
        {
            sock.nonBlockRead(buf);
            if (parser.update(buf.data, buf.readSize))
            {
                ;
            }
        }
    }
};

}
