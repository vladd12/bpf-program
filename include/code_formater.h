#pragma once

#include <fstream>
#include <string>

class SourceCodeFormater
{
private:
    std::string sourceCode;

public:
    SourceCodeFormater() noexcept = default;
    SourceCodeFormater(const std::string &src) noexcept;
    SourceCodeFormater(std::ifstream &srcFile) noexcept;

    void setSourceCode(const std::string &newSrc);
    const std::string &getSourceCode() const noexcept;
};
