#pragma once

#include <bpf_core/iec/iec_parser.h>
#include <bpf_core/iec/validator.h>
#include <bpf_core/utils/fast_file.h>
#include <bpf_core/utils/socket.h>
#include <iostream>

namespace net
{

class BPFDriver
{
private:
    utils::Buffer buf;
    utils::Socket sock;
    utils::FastFile output;
    iec::IecParser parser;
    iec::Validator validator;

public:
    explicit BPFDriver() = delete;
    explicit BPFDriver(const BPFDriver &rhs) = delete;
    explicit BPFDriver(BPFDriver &&rhs) = delete;
    BPFDriver &operator=(const BPFDriver &rhs) = delete;
    BPFDriver &operator=(BPFDriver &&rhs) = delete;

    explicit BPFDriver(const utils::Socket &socket, const std::string &filename) : sock(socket), output(filename)
    {
        buf.data = new u8[buf.allocSize];
    }

    ~BPFDriver()
    {
        delete[] buf.data;
        sock.closeSock();
    }

    void run()
    {
        while (true)
        {
            sock.nonBlockRead(buf);
            if (parser.update(buf.data, buf.readSize))
            {
                auto sequence = parser.parse();
                validator.update(sequence);

                auto curr = sequence.data[0].smpCnt;
                printf("Count: %04X\n", curr);

                if (sequence.data != nullptr)
                    delete[] sequence.data;
            }
        }
    }
};

}
