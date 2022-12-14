#pragma once

#include <bcc/BPF.h>
#include <common.h>

class BpfWrapper
{
private:
    std::unique_ptr<ebpf::BPF> bpfPtr;
    std::string bpfProg;

    /// \brief Reading file specified by filepath in std::string.
    std::string read_file(const std::string_view &filepath);

    /// \brief Replacing all substrings "what" with "with".
    std::size_t replace_all(std::string &inout, std::string_view what, std::string_view with);

    /// \brief Removing all substrings "what".
    std::size_t remove_all(std::string &inout, std::string_view what);

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

    ///
    ebpf::StatusTuple filterProgText(const std::string &iface, const std::string &srcMac, const std::string &svID);

    /// \brief Returns eBPF object stored in ebpf::BPF smart pointer.
    ebpf::BPF *getBpfObject();

    /// \brief Attaching raw socket for listening ethernet interface, specified by ifaceName.
    ebpf::StatusTuple attachRawSocket(const std::string &ifaceName, const int function, int &socket);
};
