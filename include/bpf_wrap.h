#pragma once

#include <bcc/BPF.h>
#include <common.h>

class BpfWrapper
{
private:
    std::unique_ptr<ebpf::BPF> bpfPtr;
    std::string bpfProg;

    void filter_iface_mac(const std::string &ifaceName);
    void filter_src_mac(const std::string &srcMac);
    void filter_sv_id(const std::string &svID);

    /// \brief Getting a raw socket and binding it to interface, specified by ifaceName.
    int get_raw_socket(const std::string &ifaceName);

public:
    /// \brief Default c-tor.
    explicit BpfWrapper();

    /// \brief Initializations eBPF program from file specified by programPath.
    void initByFile(const std::string &programPath);

    /// \brief Filtering BPF program source code.
    ebpf::StatusTuple filterProgText(const std::string &iface, const std::string &srcMac, const std::string &svID);

    /// \brief Returns eBPF object stored in ebpf::BPF smart pointer.
    ebpf::BPF *getBpfObject();

    /// \brief Attaching raw socket for listening ethernet interface, specified by ifaceName.
    ebpf::StatusTuple attachRawSocket(const std::string &ifaceName, const int function, int &socket);
};
