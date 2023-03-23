#include <utils.h>

std::string util::read_file(std::ifstream &stream)
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

std::string util::read_file(const std::string_view &filepath)
{
    auto stream = std::ifstream(filepath.data());
    auto out = util::read_file(stream);
    stream.close();
    return out;
}

std::string util::replace_all(const std::string &in, const std::string_view what, const std::string_view with)
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

std::string util::remove_all(const std::string &in, const std::string_view what)
{
    return util::replace_all(in, what, "");
}

std::string util::get_mac_by_iface_name(const std::string &ifaceName)
{
    if (!ifaceName.empty())
    {
        auto deviceAddressFilepath = "/sys/class/net/" + ifaceName + "/address";
        auto addressText = util::read_file(deviceAddressFilepath);
        util::remove_all(addressText, ":");
        util::remove_all(addressText, "\n");
        return "0x" + addressText;
    }
    else
        return "0xFFFFFFFFFFFF";
}
