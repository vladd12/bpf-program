#include "iec_core/utils/fast_file.h"

namespace utils
{

FastFile::FastFile(const char *filename) : file(fopen(filename, "a"))
{
    fseek(file.get(), 0, SEEK_END);
}

FastFile::FastFile(const std::string &filename) : FastFile(filename.c_str())
{
}

bool FastFile::write(const char ch)
{
    const char array[] = { ch, '\0' };
    return write(&array[0]);
}

bool FastFile::write(const char *text)
{
    auto result = fputs(text, file.get());
    return result != EOF;
}

bool FastFile::write(const std::string &text)
{
    return write(text.c_str());
}

} // namespace utils
