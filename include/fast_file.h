#pragma once

#include <cstdio>
#include <memory>
#include <string>

struct FileCloser
{
    void operator()(FILE *fHandle)
    {
        fclose(fHandle);
    }
};

using FilePtr = std::unique_ptr<FILE, FileCloser>;

class FastFile
{
private:
    FilePtr file;

public:
    explicit FastFile(const char *filename);
    explicit FastFile(const std::string &filename);

    bool write(const char ch);
    bool write(const char *text);
    bool write(const std::string &text);

    template <typename T> //
    bool write(const T number)
    {
        return write(std::to_string(number) + '\n');
    }
};
