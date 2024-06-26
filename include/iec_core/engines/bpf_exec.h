#pragma once

#include <bcc/BPF.h>
#include <iec_core/utils/code_formater.h>

/// \brief Class for compiling and loading BPF program.
class BPFExecutor
{
private:
    std::unique_ptr<ebpf::BPF> bpfPtr;
    utils::SourceCodeFormater bpfProg;

    /// \brief Attaching raw socket for listening ethernet interface, specified by ifaceName.
    ebpf::StatusTuple attachRawSocket(const std::string &ifaceName, const int function, int &socket);

public:
    /// \brief Default c-tor.
    explicit BPFExecutor(const std::string &programPath);

    /// \brief Getting a raw socket and binding it to interface, specified by ifaceName.
    static int getRawSocket(const std::string &ifaceName);

    /// \brief Filtering BPF program source code.
    void filterSourceCode(const std::string &ifaceName, const std::string &srcMac, const std::string &svID);

    /// \brief Running BPF program.
    ebpf::StatusTuple load();

    /// \brief Getting device socket, that associated with the BPF program function.
    /// \param sock_fd [in, out] - the associated with BPF function socket.
    /// \param functionName [in] - name of the executed function of BPF program.
    /// \param ifaceName [in] - name of the network interface, using for creating the device socket.
    /// \return Status of creating and attaching the raw socket to the associated function.
    ebpf::StatusTuple getDeviceSocket(int &sock_fd, const std::string &functionName, const std::string &ifaceName);
};
