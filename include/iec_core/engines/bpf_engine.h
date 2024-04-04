#pragma once

#include <iec_core/iec/iec_parser.h>
#include <iec_core/iec/validator.h>
#include <iec_core/utils/fast_file.h>
#include <iec_core/utils/socket.h>
#include <iostream>

namespace engines
{

class BPFEngine
{
private:
    utils::Buffer buf;
    utils::Socket sock;
    utils::FastFile output;
    iec::IecParser parser;
    iec::Validator validator;

public:
    explicit BPFEngine() = delete;
    explicit BPFEngine(const BPFEngine &rhs) = delete;
    explicit BPFEngine(BPFEngine &&rhs) = delete;
    BPFEngine &operator=(const BPFEngine &rhs) = delete;
    BPFEngine &operator=(BPFEngine &&rhs) = delete;

    explicit BPFEngine(const utils::Socket &socket, const std::string &filename) : sock(socket), output(filename)
    {
        buf.data = new u8[buf.allocSize];
    }

    ~BPFEngine()
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

} // namespace engines
