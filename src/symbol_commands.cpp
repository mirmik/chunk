#include "symbol_commands.h"
#include "symbol_command_objects.h"

#include "command_parse_helpers.h"
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

} // namespace

namespace commands_detail
{
    RegionReplaceCommand::RegionReplaceCommand(const std::string &name)
        : Command(name)
    {
    }

    void RegionReplaceCommand::parse(const nos::trent &tr)
    {
        section_.filepath = command_parse::get_scalar(tr, "path");
        if (section_.filepath.empty())
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'path' key");

        section_.indent_from_marker = command_parse::parse_indent_from_options(
            tr, section_.indent_from_marker);
        section_.comment = command_parse::get_scalar(tr, "comment");

        std::string payload_text = command_parse::get_scalar(tr, "payload");
        if (payload_text.empty())
            throw std::runtime_error(
                "YAML patch: symbol op '" + command_name() + "' for file '" +
                section_.filepath + "' requires 'payload'");
        section_.payload = command_parse::split_scalar_lines(payload_text);
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
            section_.payload, prefix, section_.indent_from_marker);

        lines.erase(begin, end);
        lines.insert(
            lines.begin() + r.start_line, payload.begin(), payload.end());
    }

    ReplaceCppClassCommand::ReplaceCppClassCommand()
        : RegionReplaceCommand("replace-cpp-class")
    {
    }

    void ReplaceCppClassCommand::parse(const nos::trent &tr)
    {
        RegionReplaceCommand::parse(tr);

        std::string class_text = command_parse::get_scalar(tr, "class");
        if (class_text.empty())
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'class' key");
        section_.arg1 = class_text;
    }

    bool ReplaceCppClassCommand::find_region(const std::string &text,
                                             Region &r) const
    {
        CppSymbolFinder finder(text);
        return finder.find_class(section_.arg1, r);
    }

    std::string ReplaceCppClassCommand::not_found_error() const
    {
        return "replace-cpp-class: class not found: " + section_.arg1 +
               " in file: " + section_.filepath;
    }

    std::string ReplaceCppClassCommand::invalid_region_error() const
    {
        return "replace-cpp-class: invalid region";
    }

    ReplaceCppMethodCommand::ReplaceCppMethodCommand()
        : RegionReplaceCommand("replace-cpp-method")
    {
    }

    void ReplaceCppMethodCommand::parse(const nos::trent &tr)
    {
        RegionReplaceCommand::parse(tr);

        std::string class_text = command_parse::get_scalar(tr, "class");
        std::string method_text = command_parse::get_scalar(tr, "method");
        std::string symbol_text = command_parse::get_scalar(tr, "symbol");

        if (!class_text.empty() && !method_text.empty())
        {
            cls = class_text;
            method = method_text;
        }
        else if (!symbol_text.empty())
        {
            auto pos = symbol_text.find("::");
            if (pos == std::string::npos)
                throw std::runtime_error(
                    "replace-cpp-method: expected 'Class::method' or "
                    "'Class method'");
            cls = symbol_text.substr(0, pos);
            method = symbol_text.substr(pos + 2);
        }
        else
        {
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'class'+'method' or 'symbol'");
        }

        if (cls.empty() || method.empty())
            throw std::runtime_error(
                "replace-cpp-method: empty class or method name");
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
               " in file: " + section_.filepath;
    }

    std::string ReplaceCppMethodCommand::invalid_region_error() const
    {
        return "replace-cpp-method: invalid region";
    }

    ReplacePyClassCommand::ReplacePyClassCommand()
        : RegionReplaceCommand("replace-py-class")
    {
    }

    void ReplacePyClassCommand::parse(const nos::trent &tr)
    {
        RegionReplaceCommand::parse(tr);

        std::string class_text = command_parse::get_scalar(tr, "class");
        if (class_text.empty())
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'class' key");
        section_.arg1 = class_text;
    }

    bool ReplacePyClassCommand::find_region(const std::string &text,
                                            Region &r) const
    {
        PythonSymbolFinder finder(text);
        return finder.find_class(section_.arg1, r);
    }

    std::string ReplacePyClassCommand::not_found_error() const
    {
        return "replace-py-class: class not found: " + section_.arg1 +
               " in file: " + section_.filepath;
    }

    std::string ReplacePyClassCommand::invalid_region_error() const
    {
        return "replace-py-class: invalid region";
    }

    ReplacePyMethodCommand::ReplacePyMethodCommand()
        : RegionReplaceCommand("replace-py-method")
    {
    }

    void ReplacePyMethodCommand::parse(const nos::trent &tr)
    {
        RegionReplaceCommand::parse(tr);

        std::string class_text = command_parse::get_scalar(tr, "class");
        std::string method_text = command_parse::get_scalar(tr, "method");
        std::string symbol_text = command_parse::get_scalar(tr, "symbol");

        if (!class_text.empty() && !method_text.empty())
        {
            cls = class_text;
            method = method_text;
        }
        else if (!symbol_text.empty())
        {
            auto pos = symbol_text.find('.');
            if (pos == std::string::npos)
                throw std::runtime_error(
                    "replace-py-method: expected 'Class.method' or "
                    "'Class method'");
            cls = symbol_text.substr(0, pos);
            method = symbol_text.substr(pos + 1);
        }
        else
        {
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'class'+'method' or 'symbol'");
        }

        if (cls.empty() || method.empty())
            throw std::runtime_error(
                "replace-py-method: empty class or method name");
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
               " in file: " + section_.filepath;
    }

    std::string ReplacePyMethodCommand::invalid_region_error() const
    {
        return "replace-py-method: invalid region";
    }
} // namespace commands_detail
