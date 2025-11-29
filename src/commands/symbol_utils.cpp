#include "commands/symbol_utils.h"

std::string extract_indent_prefix(const std::vector<std::string> &lines,
                                         int index)
{
    if (index < 0 || index >= static_cast<int>(lines.size()))
        return {};

    const std::string &line = lines[static_cast<std::size_t>(index)];
    std::size_t j = 0;
    while (j < line.size() && (line[j] == ' ' || line[j] == '\t'))
        ++j;

    return std::string(line, 0, j);
}

std::vector<std::string>
apply_indent_prefix(const std::vector<std::string> &payload,
                    const std::string &prefix,
                    bool enabled)
{
    if (!enabled || prefix.empty())
        return payload;

    std::vector<std::string> result;
    result.reserve(payload.size());
    for (const auto &ln : payload)
    {
        if (ln.empty())
            result.push_back(ln);
        else
            result.push_back(prefix + ln);
    }
    return result;
}

std::string join_lines(const std::vector<std::string> &lines)
{
    if (lines.empty())
        return std::string();

    std::string text;
    std::size_t total = 0;
    for (const auto &s : lines)
        total += s.size() + 1;
    text.reserve(total);

    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        text += lines[i];
        if (i + 1 < lines.size())
            text += '\n';
    }

    return text;
}
