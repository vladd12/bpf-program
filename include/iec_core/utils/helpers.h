#pragma once

#include <cstdint>
#include <fstream>
#include <linux/if_ether.h>
#include <string>

namespace utils
{

/// \brief Reading the file specified by the file stream to the string.
std::string read_file(std::ifstream &stream);

/// \brief Reading the file specified by the filepath to the string.
std::string read_file(const std::string_view &filepath);

/// \brief Replacing all substrings "what" with "with".
std::string replace_all(const std::string &in, const std::string_view what, const std::string_view with);

/// \brief Removing all substrings "what".
std::string remove_all(const std::string &in, const std::string_view what);

/// \brief Getting the device's MAC address by the name of ethernet interface.
std::string get_mac_by_iface_name(const std::string &ifaceName);

/// \brief Printing ethernet MAC address to the standard output stream.
void printMacAddress(const std::uint8_t mac[]);

}
