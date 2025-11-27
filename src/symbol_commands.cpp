#include "symbol_commands.h"

#include "symbols.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace
{
std::string extract_indent_prefix(const std::vector<std::string> &lines,
                                  int index)
{
    if (index < 0 || index >= static_cast<int>(lines.size()))
        return {};

    const std::string &line = lines[static_cast<std::size_t>(index)];
    std::size_t j = 0;
    while (j < line.size() &&
           (line[j] == ' ' || line[j] == '\t'))
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

std::pair<std::string, std::string> parse_cpp_target(const Section &s)
{
    if (!s.arg2.empty())
        return {s.arg1, s.arg2};

    auto pos = s.arg1.find("::");
    if (pos == std::string::npos)
        throw std::runtime_error(
            "replace-cpp-method: expected 'Class::method' or 'Class method'");

    std::string cls = s.arg1.substr(0, pos);
    std::string method = s.arg1.substr(pos + 2);
    if (cls.empty() || method.empty())
        throw std::runtime_error(
            "replace-cpp-method: empty class or method name");

    return {cls, method};
}

std::pair<std::string, std::string> parse_python_target(const Section &s)
{
    if (!s.arg2.empty())
        return {s.arg1, s.arg2};

    auto pos = s.arg1.find('.');
    if (pos == std::string::npos)
        throw std::runtime_error(
            "replace-py-method: expected 'Class.method' or 'Class method'");

    std::string cls = s.arg1.substr(0, pos);
    std::string method = s.arg1.substr(pos + 1);
    if (cls.empty() || method.empty())
        throw std::runtime_error(
            "replace-py-method: empty class or method name");

    return {cls, method};
}

void replace_cpp_class(const Section &s,
                       const std::string &filepath,
                       const std::string &text,
                       std::vector<std::string> &lines)
{
    if (s.arg1.empty())
        throw std::runtime_error(
            "replace-cpp-class: missing class name for file: " +
            filepath);

    CppSymbolFinder finder(text);
    Region r;
    if (!finder.find_class(s.arg1, r))
        throw std::runtime_error(
            "replace-cpp-class: class not found: " + s.arg1 +
            " in file: " + filepath);

    if (r.start_line < 0 || r.end_line < r.start_line ||
        r.end_line >= static_cast<int>(lines.size()))
        throw std::runtime_error("replace-cpp-class: invalid region");

    auto begin = lines.begin() + r.start_line;
    auto end   = lines.begin() + (r.end_line + 1);

    std::string prefix = extract_indent_prefix(lines, r.start_line);
    std::vector<std::string> payload =
        apply_indent_prefix(s.payload, prefix, s.indent_from_marker);

    lines.erase(begin, end);
    lines.insert(lines.begin() + r.start_line,
                 payload.begin(),
                 payload.end());
}

void replace_cpp_method(const Section &s,
                        const std::string &filepath,
                        const std::string &text,
                        std::vector<std::string> &lines)
{
    auto [cls, method] = parse_cpp_target(s);

    CppSymbolFinder finder(text);
    Region r;
    if (!finder.find_method(cls, method, r))
        throw std::runtime_error(
            "replace-cpp-method: method not found: " + cls +
            "::" + method + " in file: " + filepath);

    if (r.start_line < 0 || r.end_line < r.start_line ||
        r.end_line >= static_cast<int>(lines.size()))
        throw std::runtime_error("replace-cpp-method: invalid region");

    auto begin = lines.begin() + r.start_line;
    auto end   = lines.begin() + (r.end_line + 1);

    std::string prefix = extract_indent_prefix(lines, r.start_line);
    std::vector<std::string> payload =
        apply_indent_prefix(s.payload, prefix, s.indent_from_marker);

    lines.erase(begin, end);
    lines.insert(lines.begin() + r.start_line,
                 payload.begin(),
                 payload.end());
}

void replace_py_class(const Section &s,
                      const std::string &filepath,
                      const std::string &text,
                      std::vector<std::string> &lines)
{
    if (s.arg1.empty())
        throw std::runtime_error(
            "replace-py-class: missing class name for file: " +
            filepath);

    PythonSymbolFinder finder(text);
    Region r;
    if (!finder.find_class(s.arg1, r))
        throw std::runtime_error(
            "replace-py-class: class not found: " + s.arg1 +
            " in file: " + filepath);

    if (r.start_line < 0 || r.end_line < r.start_line ||
        r.end_line >= static_cast<int>(lines.size()))
        throw std::runtime_error("replace-py-class: invalid region");

    auto begin = lines.begin() + r.start_line;
    auto end   = lines.begin() + (r.end_line + 1);

    std::string prefix = extract_indent_prefix(lines, r.start_line);
    std::vector<std::string> payload =
        apply_indent_prefix(s.payload, prefix, s.indent_from_marker);

    lines.erase(begin, end);
    lines.insert(lines.begin() + r.start_line,
                 payload.begin(),
                 payload.end());
}

void replace_py_method(const Section &s,
                       const std::string &filepath,
                       const std::string &text,
                       std::vector<std::string> &lines)
{
    auto [cls, method] = parse_python_target(s);

    PythonSymbolFinder finder(text);
    Region r;
    if (!finder.find_method(cls, method, r))
        throw std::runtime_error(
            "replace-py-method: method not found: " + cls + "." +
            method + " in file: " + filepath);

    if (r.start_line < 0 || r.end_line < r.start_line ||
        r.end_line >= static_cast<int>(lines.size()))
        throw std::runtime_error("replace-py-method: invalid region");

    auto begin = lines.begin() + r.start_line;
    auto end   = lines.begin() + (r.end_line + 1);

    std::string prefix = extract_indent_prefix(lines, r.start_line);
    std::vector<std::string> payload =
        apply_indent_prefix(s.payload, prefix, s.indent_from_marker);

    lines.erase(begin, end);
    lines.insert(lines.begin() + r.start_line,
                 payload.begin(),
                 payload.end());
}
} // namespace

void apply_symbol_commands(const std::string &filepath,
                           std::vector<std::string> &lines,
                           const std::vector<const Section *> &sections)
{
    for (const Section *s : sections)
    {
        std::string text = join_lines(lines);

        if (s->command == "replace-cpp-class")
        {
            replace_cpp_class(*s, filepath, text, lines);
            continue;
        }
        if (s->command == "replace-cpp-method")
        {
            replace_cpp_method(*s, filepath, text, lines);
            continue;
        }
        if (s->command == "replace-py-class")
        {
            replace_py_class(*s, filepath, text, lines);
            continue;
        }
        if (s->command == "replace-py-method")
        {
            replace_py_method(*s, filepath, text, lines);
            continue;
        }

        throw std::runtime_error(
            "apply_symbol_commands: unknown command: " + s->command);
    }
}
