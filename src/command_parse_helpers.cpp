#include "command_parse_helpers.h"

#include <stdexcept>

using nos::trent;

namespace command_parse
{
std::string get_scalar(const trent &node, const char *key)
{
    const auto &dict = node.as_dict();
    auto it = dict.find(key);
    if (it == dict.end())
        return std::string();

    const trent &v = it->second;
    if (v.is_nil())
        return std::string();

    return v.as_string();
}

std::vector<std::string> split_scalar_lines(const std::string &text)
{
    std::vector<std::string> result;
    std::size_t start = 0;

    while (start < text.size())
    {
        auto pos = text.find('\n', start);
        if (pos == std::string::npos)
        {
            result.emplace_back(text.substr(start));
            break;
        }
        result.emplace_back(text.substr(start, pos - start));
        start = pos + 1;
    }

    return result;
}

bool parse_indent_from_options(const trent &node, bool default_value)
{
    const auto &dict = node.as_dict();
    auto it_opts = dict.find("options");
    if (it_opts == dict.end() || it_opts->second.is_nil())
        return default_value;

    const auto &opts = it_opts->second.as_dict();
    auto it_ind = opts.find("indent");
    if (it_ind == opts.end())
        return default_value;

    std::string mode = it_ind->second.as_string();
    if (mode == "none" || mode == "as-is")
        return false;
    if (mode == "from-marker" || mode == "marker" || mode == "auto")
        return true;

    throw std::runtime_error("YAML patch: unknown indent mode: " + mode);
}
} // namespace command_parse
