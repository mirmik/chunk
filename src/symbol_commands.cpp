#include "symbol_commands.h"
#include "symbol_command_objects.h"

#include "symbols.h"

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
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

    std::pair<std::string, std::string> parse_cpp_target(const Section &s)
    {
        if (!s.arg2.empty())
            return {s.arg1, s.arg2};

        auto pos = s.arg1.find("::");
        if (pos == std::string::npos)
            throw std::runtime_error("replace-cpp-method: expected "
                                     "'Class::method' or 'Class method'");

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

} // namespace

namespace symbol_commands_detail
{
    RegionReplaceCommand::RegionReplaceCommand(const Section &s,
                                               std::string path)
        : section(s), filepath(std::move(path))
    {
    }

    void RegionReplaceCommand::execute(std::vector<std::string> &lines)
    {
        std::string text = join_lines(lines);
        Region r;
        if (!find_region(text, r))
            throw std::runtime_error(not_found_error());

        if (r.start_line < 0 || r.end_line < r.start_line ||
            r.end_line >= static_cast<int>(lines.size()))
            throw std::runtime_error(invalid_region_error());

        auto begin = lines.begin() + r.start_line;
        auto end = lines.begin() + (r.end_line + 1);

        std::string prefix = extract_indent_prefix(lines, r.start_line);
        std::vector<std::string> payload = apply_indent_prefix(
            section.payload, prefix, section.indent_from_marker);

        lines.erase(begin, end);
        lines.insert(
            lines.begin() + r.start_line, payload.begin(), payload.end());
    }

    ReplaceCppClassCommand::ReplaceCppClassCommand(const Section &s,
                                                   std::string path)
        : RegionReplaceCommand(s, std::move(path))
    {
        if (section.arg1.empty())
            throw std::runtime_error(
                "replace-cpp-class: missing class name for file: " + filepath);
    }

    bool ReplaceCppClassCommand::find_region(const std::string &text,
                                             Region &r) const
    {
        CppSymbolFinder finder(text);
        return finder.find_class(section.arg1, r);
    }

    std::string ReplaceCppClassCommand::not_found_error() const
    {
        return "replace-cpp-class: class not found: " + section.arg1 +
               " in file: " + filepath;
    }

    std::string ReplaceCppClassCommand::invalid_region_error() const
    {
        return "replace-cpp-class: invalid region";
    }

    ReplaceCppMethodCommand::ReplaceCppMethodCommand(const Section &s,
                                                     std::string path)
        : RegionReplaceCommand(s, std::move(path))
    {
        std::tie(cls, method) = parse_cpp_target(s);
    }

    bool ReplaceCppMethodCommand::find_region(const std::string &text,
                                              Region &r) const
    {
        CppSymbolFinder finder(text);
        return finder.find_method(cls, method, r);
    }

    std::string ReplaceCppMethodCommand::not_found_error() const
    {
        return "replace-cpp-method: method not found: " + cls + "::" + method +
               " in file: " + filepath;
    }

    std::string ReplaceCppMethodCommand::invalid_region_error() const
    {
        return "replace-cpp-method: invalid region";
    }

    ReplacePyClassCommand::ReplacePyClassCommand(const Section &s,
                                                 std::string path)
        : RegionReplaceCommand(s, std::move(path))
    {
        if (section.arg1.empty())
            throw std::runtime_error(
                "replace-py-class: missing class name for file: " + filepath);
    }

    bool ReplacePyClassCommand::find_region(const std::string &text,
                                            Region &r) const
    {
        PythonSymbolFinder finder(text);
        return finder.find_class(section.arg1, r);
    }

    std::string ReplacePyClassCommand::not_found_error() const
    {
        return "replace-py-class: class not found: " + section.arg1 +
               " in file: " + filepath;
    }

    std::string ReplacePyClassCommand::invalid_region_error() const
    {
        return "replace-py-class: invalid region";
    }

    ReplacePyMethodCommand::ReplacePyMethodCommand(const Section &s,
                                                   std::string path)
        : RegionReplaceCommand(s, std::move(path))
    {
        std::tie(cls, method) = parse_python_target(s);
    }

    bool ReplacePyMethodCommand::find_region(const std::string &text,
                                             Region &r) const
    {
        PythonSymbolFinder finder(text);
        return finder.find_method(cls, method, r);
    }

    std::string ReplacePyMethodCommand::not_found_error() const
    {
        return "replace-py-method: method not found: " + cls + "." + method +
               " in file: " + filepath;
    }

    std::string ReplacePyMethodCommand::invalid_region_error() const
    {
        return "replace-py-method: invalid region";
    }
} // namespace symbol_commands_detail

namespace
{
    const std::unordered_map<std::string,
                             symbol_commands_detail::CommandFactory> &
    symbol_command_registry()
    {
        using namespace symbol_commands_detail;

        static const std::unordered_map<std::string, CommandFactory>
            registry = {
                {"replace-cpp-class",
                 [](const Section &s, const std::string &filepath) {
                     return std::make_unique<ReplaceCppClassCommand>(s,
                                                                     filepath);
                 }},
                {"replace-cpp-method",
                 [](const Section &s, const std::string &filepath) {
                     return std::make_unique<ReplaceCppMethodCommand>(s,
                                                                      filepath);
                 }},
                {"replace-py-class",
                 [](const Section &s, const std::string &filepath) {
                     return std::make_unique<ReplacePyClassCommand>(s,
                                                                    filepath);
                 }},
                {"replace-py-method",
                 [](const Section &s, const std::string &filepath) {
                     return std::make_unique<ReplacePyMethodCommand>(s,
                                                                     filepath);
                 }},
            };

        return registry;
    }
} // namespace

std::unique_ptr<Command>
create_symbol_command(const Section &section, const std::string &filepath)
{
    const auto &registry = symbol_command_registry();
    auto it = registry.find(section.command);
    if (it == registry.end())
        throw std::runtime_error(
            "apply_symbol_commands: unknown command: " + section.command);
    return it->second(section, filepath);
}

void apply_symbol_commands(const std::string &filepath,
                           std::vector<std::string> &lines,
                           const std::vector<const Section *> &sections)
{
    for (const Section *s : sections)
    {
        auto cmd = create_symbol_command(*s, filepath);
        cmd->execute(lines);
    }
}
