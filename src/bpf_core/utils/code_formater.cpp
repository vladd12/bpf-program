#include "bpf_core/utils/code_formater.h"

#include <bpf_core/utils/helpers.h>

namespace utils
{

SourceCodeFormater::SourceCodeFormater(const std::string_view &srcFilepath) : sourceCode(read_file(srcFilepath))
{
}

SourceCodeFormater::SourceCodeFormater(std::ifstream &srcFile) : sourceCode(read_file(srcFile))
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

void SourceCodeFormater::replace(const std::string_view match, const std::string_view replace)
{
    sourceCode = replace_all(sourceCode, match, replace);
}

void SourceCodeFormater::remove(const std::string_view match)
{
    sourceCode = remove_all(sourceCode, match);
}

} // namespace utils
