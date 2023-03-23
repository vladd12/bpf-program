#pragma once

#include <fstream>
#include <string>

/// \brief Class for formatting BPF source code.
class SourceCodeFormater
{
private:
    std::string sourceCode;

public:
    explicit SourceCodeFormater() noexcept = default;
    explicit SourceCodeFormater(const std::string &src) noexcept;
    explicit SourceCodeFormater(std::ifstream &srcFile);

    /// \brief Setting new source code as string.
    void setSourceCode(const std::string &newSrc) noexcept;
    /// \brief Getting current source code as string.
    const std::string &getSourceCode() const noexcept;

    /// \brief Replace all occurrences of "match" string in the source code with "replace" string.
    void replace(const std::string &match, const std::string &replace);
    /// \brief Remove all occurrences of "match" string in the source code.
    void remove(const std::string &match);
};
