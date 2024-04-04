#pragma once

#include <cstdio>
#include <memory>
#include <string>

/// \brief Structure for closing the captured file's handle.
struct FileCloser
{
    void operator()(FILE *fHandle)
    {
        fclose(fHandle);
    }
};

using FilePtr = std::unique_ptr<FILE, FileCloser>; ///< Container for the FILE pointer.

/// \brief Class for creating file objects with fast writing data.
class FastFile
{
private:
    FilePtr file;

public:
    /// \brief Open a file with the specified filename.
    explicit FastFile(const char *filename);

    /// \brief Open a file with the specified filename.
    explicit FastFile(const std::string &filename);

    /// \brief Writing single character to rhe file.
    bool write(const char ch);

    /// \brief Writing C-style string data to the file.
    bool write(const char *text);

    /// \brief Writing std::string object's data to the file.
    bool write(const std::string &text);

    /// \brief Writing numerical data to the file.
    template <typename T> //
    bool write(const T number)
    {
        return write(std::to_string(number) + '\n');
    }
};
