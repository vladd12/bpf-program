#include <code_formater.h>
#include <utils.h>

SourceCodeFormater::SourceCodeFormater(const std::string &src) noexcept : sourceCode(src)
{
}

SourceCodeFormater::SourceCodeFormater(std::ifstream &srcFile) noexcept : sourceCode(util::read_file(srcFile))
{
}

void SourceCodeFormater::setSourceCode(const std::string &newSrc)
{
    sourceCode = newSrc;
}

const std::string &SourceCodeFormater::getSourceCode() const noexcept
{
    return sourceCode;
}
