#pragma once

#include <bcc/BPF.h>
#include <code_formater.h>
#include <common.h>

class BpfWrapper
{
private:
    std::unique_ptr<ebpf::BPF> bpfPtr;
    SourceCodeFormater bpfProg;

    /// \brief Getting a raw socket and binding it to interface, specified by ifaceName.
    int getRawSocket(const std::string &ifaceName);

    /// \brief Attaching raw socket for listening ethernet interface, specified by ifaceName.
    ebpf::StatusTuple attachRawSocket(const std::string &ifaceName, const int function, int &socket);

public:
    /// \brief Default c-tor.
    explicit BpfWrapper(const std::string &programPath);

    /// \brief Filtering BPF program source code.
    void filterSourceCode(const std::string &ifaceName, const std::string &srcMac, const std::string &svID);

    /// \brief Running BPF program.
    ebpf::StatusTuple run();

    ebpf::StatusTuple getDeviceSocket(int &sock_fd, const std::string &functionName, const std::string &ifaceName);
};
