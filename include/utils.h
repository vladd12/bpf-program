#pragma once

#include <fstream>
#include <string>

namespace util
{

/// \brief Reading the file specified by the file stream to the string.
std::string read_file(std::ifstream &stream);

/// \brief Reading the file specified by the filepath to the string.
std::string read_file(const std::string_view &filepath);

/// \brief Replacing all substrings "what" with "with".
std::string replace_all(const std::string &in, const std::string_view what, const std::string_view with);

/// \brief Removing all substrings "what".
std::string remove_all(const std::string &in, const std::string_view what);

std::string get_mac_by_iface_name();

}
