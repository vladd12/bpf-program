#include "iec_core/engines/bpf_engine.h"

#include <iec_core/engines/bpf_exec.h>
#include <stdexcept>

namespace engines
{

BPFEngine::BPFEngine(const std::string &filename) : output(filename)
{
    buf.data = new u8[buf.allocSize];
}

BPFEngine::BPFEngine(const utils::Socket &socket, const std::string &filename) : BPFEngine(filename)
{
    sock = socket;
}

BPFEngine::BPFEngine(const BPFExecutor &executor, const std::string &filename) : BPFEngine(filename)
{
    (void)(executor);
}

BPFEngine::~BPFEngine()
{
    delete[] buf.data;
    sock.closeSock();
}

void BPFEngine::run()
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

BPFEngine BPFEngine::create(const std::string_view &ifaceName, const std::string_view &srcMacAddr, const std::string_view &svID)
{
    BPFExecutor executor("bpf/ethernet-parse.c");
    executor.filterSourceCode(ifaceName.data(), srcMacAddr.data(), svID.data());
    auto status = executor.load();
    if (status.ok())
    {
        int sock = -1;
        status = executor.getDeviceSocket(sock, "iec61850_filter", ifaceName.data());
        if (status.ok() && sock >= 0)
            return BPFEngine(utils::Socket(sock), "out.txt");
    }
    throw std::runtime_error("Can't compile or load BPF program");
}

} // namespace engines