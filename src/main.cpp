#include <bpf_exec.h>
#include <exception>
#include <fast_file.h>
#include <iec_parser.h>
#include <iostream>
#include <thread>
#include <utils.h>

// Linux
#include <net/ethernet.h>
#include <sys/select.h>
#include <sys/socket.h>

using byte = std::uint8_t;
using word = std::uint16_t;
using dword = std::uint32_t;

int main()
{
    auto bpf = std::unique_ptr<BpfExec>(new BpfExec("bpf/ethernet-parse.c"));
    auto ifaceName = std::string("enp0s8");          // enp0s8 - VM, eth0 - hardware
    auto srcMacAddr = std::string("0x0cefaf3042cc"); // 0x0cefaf3042cc - 80p, 0x0cefaf3042cd - 256p
    auto svID = std::string("ENS80pointMU01");       // ENS80pointMU01 - 80p, ENS256MUnn01 - 256p
    bpf->filterSourceCode(ifaceName, srcMacAddr, svID);
    auto status = bpf->run();
    if (status.ok())
    {
        status = bpf->foo();
        if (status.ok())
        {
            ;
        }
    }
    util::printStatusMessage(status);
    return 0;
}
