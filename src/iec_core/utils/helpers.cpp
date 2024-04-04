#include "iec_core/utils/helpers.h"

#include <iostream>

namespace utils
{

std::string read_file(std::ifstream &stream)
{
    constexpr auto read_size = std::size_t(4096);
    stream.exceptions(std::ios_base::badbit);

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(&buf[0], read_size))
        out.append(buf, 0, stream.gcount());

    out.append(buf, 0, stream.gcount());
    return out;
}

std::string read_file(const std::string_view &filepath)
{
    auto stream = std::ifstream(filepath.data());
    auto out = read_file(stream);
    stream.close();
    return out;
}

std::string replace_all(const std::string &in, const std::string_view what, const std::string_view with)
{
    auto out = std::string(in);
    for (auto pos = std::string::size_type {};                          //
         out.npos != (pos = out.find(what.data(), pos, what.length())); //
         pos += with.length())                                          //
    {
        out.replace(pos, what.length(), with.data(), with.length());
    }
    return out;
}

std::string remove_all(const std::string &in, const std::string_view what)
{
    return replace_all(in, what, "");
}

std::string get_mac_by_iface_name(const std::string &ifaceName)
{
    if (!ifaceName.empty())
    {
        auto deviceAddressFilepath = "/sys/class/net/" + ifaceName + "/address";
        auto addressText = read_file(deviceAddressFilepath);
        addressText = remove_all(addressText, ":");
        addressText = remove_all(addressText, "\n");
        return "0x" + addressText;
    }
    else
        return "0xFFFFFFFFFFFF";
}

void printMacAddress(const std::uint8_t mac[ETH_ALEN])
{
    printf("0x%02X%02X%02X%02X%02X%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

}
