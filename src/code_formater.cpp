#include <code_formater.h>
#include <utils.h>

SourceCodeFormater::SourceCodeFormater(const std::string &src) noexcept : sourceCode(src)
{
}

SourceCodeFormater::SourceCodeFormater(std::ifstream &srcFile) : sourceCode(util::read_file(srcFile))
{
}

void SourceCodeFormater::setSourceCode(const std::string &newSrc) noexcept
{
    sourceCode = newSrc;
}

const std::string &SourceCodeFormater::getSourceCode() const noexcept
{
    return sourceCode;
}

void SourceCodeFormater::replace(const std::string &match, const std::string &replace)
{
    sourceCode = util::replace_all(sourceCode, match, replace);
}

void SourceCodeFormater::remove(const std::string &match)
{
    sourceCode = util::remove_all(sourceCode, match);
}
