#pragma once

#include "command.h"
#include "command_parse_helpers.h"
#include "commands/text_utils.hpp"
#include "commands/symbol_utils.hpp"
#include "symbols.h"

// Common base for text commands that operate around markers
class MarkerTextCommand : public Command
{
public:
    explicit MarkerTextCommand(const std::string &name) : Command(name) {}

    void parse(const nos::trent &tr) override
    {
        section_.filepath = command_parse::get_scalar(tr, "path");
        if (section_.filepath.empty())
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'path' key");

        section_.indent_from_marker =
            command_parse::parse_indent_from_options(
                tr, section_.indent_from_marker);
        section_.comment = command_parse::get_scalar(tr, "comment");

        std::string marker_text = command_parse::get_scalar(tr, "marker");
        std::string before_text = command_parse::get_scalar(tr, "before");
        std::string after_text = command_parse::get_scalar(tr, "after");
        std::string payload_text = command_parse::get_scalar(tr, "payload");

        if (command_name() == "prepend-text" || command_name() == "append-text")
        {
            if (payload_text.empty())
                throw std::runtime_error(
                    "YAML patch: text op '" + command_name() + "' for file '" +
                    section_.filepath + "' requires 'payload'");
            section_.payload = command_parse::split_scalar_lines(payload_text);
            return;
        }

        if (marker_text.empty())
            throw std::runtime_error(
                "YAML patch: text op '" + command_name() + "' for file '" +
                section_.filepath + "' requires 'marker'");

        section_.marker = command_parse::split_scalar_lines(marker_text);

        if (!before_text.empty())
            section_.before = command_parse::split_scalar_lines(before_text);
        if (!after_text.empty())
            section_.after = command_parse::split_scalar_lines(after_text);
        if (command_name() != "delete-text" && !payload_text.empty())
            section_.payload = command_parse::split_scalar_lines(payload_text);
    }

    void execute(std::vector<std::string> &lines) override
    {
        using namespace text_utils;

        if (section_.marker.empty())
            throw std::runtime_error("empty marker in text command for file: " +
                                     section_.filepath);

        std::vector<MarkerMatch> matches =
            find_marker_matches(lines, section_.marker, &section_);
        if (matches.empty())
            throw std::runtime_error("text marker not found for file: " +
                                     section_.filepath + "\ncommand: " +
                                     section_.command + "\n");

        int mindex = find_best_marker_match(lines, &section_, matches);
        if (mindex < 0)
            throw std::runtime_error("cannot locate marker uniquely");

        const MarkerMatch &mm = matches[static_cast<std::size_t>(mindex)];
        if (mm.begin < 0 || mm.end < mm.begin ||
            mm.end >= static_cast<int>(lines.size()))
            throw std::runtime_error("internal error: invalid marker match range");

        std::size_t begin = static_cast<std::size_t>(mm.begin);
        std::size_t end = static_cast<std::size_t>(mm.end);

        std::vector<std::string> payload = prepare_payload(lines, begin);
        apply(lines, begin, end, payload);
    }

protected:
    virtual bool should_indent_payload() const { return false; }

private:
    std::vector<std::string>
    prepare_payload(const std::vector<std::string> &lines,
                    std::size_t begin) const
    {
        if (!should_indent_payload() || !section_.indent_from_marker)
            return section_.payload;

        std::string prefix;
        if (begin < lines.size())
        {
            const std::string &marker_line = lines[begin];
            std::size_t j = 0;
            while (j < marker_line.size() &&
                   (marker_line[j] == ' ' || marker_line[j] == '\t'))
                ++j;
            prefix.assign(marker_line, 0, j);
        }

        std::vector<std::string> adjusted;
        adjusted.reserve(section_.payload.size());
        for (const auto &ln : section_.payload)
        {
            if (ln.empty())
                adjusted.push_back(ln);
            else
                adjusted.push_back(prefix + ln);
        }
        return adjusted;
    }

    virtual void apply(std::vector<std::string> &lines,
                       std::size_t begin,
                       std::size_t end,
                       const std::vector<std::string> &payload) = 0;
};

// Base for region replace symbol commands
class RegionReplaceCommand : public Command
{
public:
    explicit RegionReplaceCommand(const std::string &name) : Command(name) {}

    void parse(const nos::trent &tr) override
    {
        section_.filepath = command_parse::get_scalar(tr, "path");
        if (section_.filepath.empty())
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'path' key");

        section_.indent_from_marker =
            command_parse::parse_indent_from_options(
                tr, section_.indent_from_marker);
        section_.comment = command_parse::get_scalar(tr, "comment");

        std::string payload_text = command_parse::get_scalar(tr, "payload");
        if (payload_text.empty())
            throw std::runtime_error("YAML patch: symbol op '" +
                                     command_name() + "' for file '" +
                                     section_.filepath +
                                     "' requires 'payload'");
        section_.payload = command_parse::split_scalar_lines(payload_text);
    }

    void execute(std::vector<std::string> &lines) override
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
        lines.insert(lines.begin() + r.start_line, payload.begin(), payload.end());
    }

protected:
    virtual bool find_region(const std::string &text, Region &r) const = 0;
    virtual std::string not_found_error() const = 0;
    virtual std::string invalid_region_error() const = 0;
};
