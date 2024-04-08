#include "iec_core/engines/bpf_engine.h"

#include <iostream>

namespace engines
{

BPFEngine::BPFEngine() : BaseEngine(), executor("bpf/ethernet-parse.c"), socket()
{
}

bool BPFEngine::setup(const EngineSettings &settings)
{
    executor.filterSourceCode(settings.iface.data(), settings.sourceMAC.data(), settings.svID.data());
    auto status = executor.load();
    if (status.ok())
    {
        int socket_fd = -1;
        status = executor.getDeviceSocket(socket_fd, "iec61850_filter", settings.iface.data());
        if (status.ok() && socket_fd >= 0)
        {
            socket.setHandle(socket_fd);
            if (!socket.setNonBlockingMode())
                std::cout << "Couldn't set non-blocking read mode for socket!\n";
            return true;
        }
    }
    std::cout << status.msg() << '\n';
    return false;
}

void BPFEngine::run()
{
    while (running)
    {
        socket.readTo(buffer);
        //        if (buffer.getSize() < 800)
        //            ;
    }
}

// void BPFEngine::run()
//{
//    while (true)
//    {
//        sock.nonBlockRead(buf);
//        if (parser.update(buf.data, buf.readSize))
//        {
//            auto sequence = parser.parse();
//            validator.update(sequence);
//            auto curr = sequence.data[0].smpCnt;
//            printf("Count: %04X\n", curr);
//            if (sequence.data != nullptr)
//                delete[] sequence.data;
//        }
//    }
//}

} // namespace engines
