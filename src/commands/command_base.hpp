#pragma once

#include "command.h"
#include "command_parse_helpers.h"
#include "commands/text_utils.h"
#include "commands/symbol_utils.h"
#include "symbols.h"

#include <ostream>

// Common base for text commands that operate around markers
class MarkerTextCommand : public Command
{
public:
    explicit MarkerTextCommand(const std::string &name) : Command(name) {}

    void parse(const nos::trent &tr) override
    {
        filepath_ = command_parse::get_scalar(tr, "path");
        if (filepath_.empty())
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'path' key");

        indent_from_marker_ =
            command_parse::parse_indent_from_options(
                tr, indent_from_marker_);
        comment_ = command_parse::get_scalar(tr, "comment");

        std::string marker_text = command_parse::get_scalar(tr, "marker");
        std::string before_text = command_parse::get_scalar(tr, "before");
        std::string after_text = command_parse::get_scalar(tr, "after");
        std::string payload_text = command_parse::get_scalar(tr, "payload");

        if (marker_text.empty())
            throw std::runtime_error(
                "YAML patch: text op '" + command_name() + "' for file '" +
                filepath_ + "' requires 'marker'");

        marker_ = command_parse::split_scalar_lines(marker_text);

        if (!before_text.empty())
            before_ = command_parse::split_scalar_lines(before_text);
        if (!after_text.empty())
            after_ = command_parse::split_scalar_lines(after_text);
        if (command_name() != "delete-text" && !payload_text.empty())
            payload_ = command_parse::split_scalar_lines(payload_text);
    }

    void execute(std::vector<std::string> &lines) override
    {
        using namespace text_utils;

        if (marker_.empty())
            throw std::runtime_error("empty marker in text command for file: " +
                                     filepath_);

        PatchLanguage lang = detect_language(language_);
        std::vector<MarkerMatch> matches =
            find_marker_matches(lines, marker_, lang);
        if (matches.empty())
            throw std::runtime_error("text marker not found for file: " +
                                     filepath_ + "\ncommand: " +
                                     command_name() + "\n");

        int mindex =
            find_best_marker_match(lines, lang, before_, after_, matches);
        if (mindex < 0)
            throw std::runtime_error("cannot locate marker uniquely");

        const MarkerMatch &mm = matches[static_cast<std::size_t>(mindex)];
        if (mm.begin < 0 || mm.end < mm.begin ||
            mm.end >= static_cast<int>(lines.size()))
            throw std::runtime_error(
                "internal error: invalid marker match range");

        std::size_t begin = static_cast<std::size_t>(mm.begin);
        std::size_t end = static_cast<std::size_t>(mm.end);

        std::vector<std::string> payload = prepare_payload(lines, begin);
        apply(lines, begin, end, payload);
    }

    void append_debug_info(std::ostream &os) const override
    {
        if (!marker_.empty())
        {
            os << "\nsection marker preview:\n";
            std::size_t max_preview_lines = 3;
            for (std::size_t i = 0;
                 i < marker_.size() && i < max_preview_lines;
                 ++i)
            {
                os << marker_[i] << "\n";
            }
        }
    }

protected:
    virtual bool should_indent_payload() const { return false; }

    bool indent_from_marker_ = true;
    std::vector<std::string> marker_;
    std::vector<std::string> before_;
    std::vector<std::string> after_;
    std::vector<std::string> payload_;

private:
    std::vector<std::string>
    prepare_payload(const std::vector<std::string> &lines,
                    std::size_t begin) const
    {
        if (!should_indent_payload() || !indent_from_marker_)
            return payload_;

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
        adjusted.reserve(payload_.size());
        for (const auto &ln : payload_)
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
        filepath_ = command_parse::get_scalar(tr, "path");
        if (filepath_.empty())
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'path' key");

        indent_from_symbol_ =
            command_parse::parse_indent_from_options(
                tr, indent_from_symbol_);
        comment_ = command_parse::get_scalar(tr, "comment");

        std::string payload_text = command_parse::get_scalar(tr, "payload");
        if (payload_text.empty())
            throw std::runtime_error("YAML patch: symbol op '" +
                                     command_name() + "' for file '" +
                                     filepath_ + "' requires 'payload'");
        payload_ = command_parse::split_scalar_lines(payload_text);
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
        std::vector<std::string> payload =
            apply_indent_prefix(payload_, prefix, indent_from_symbol_);

        lines.erase(begin, end);
        lines.insert(lines.begin() + r.start_line,
                     payload.begin(),
                     payload.end());
    }

protected:
    bool indent_from_symbol_ = true;
    std::vector<std::string> payload_;

    virtual bool find_region(const std::string &text, Region &r) const = 0;
    virtual std::string not_found_error() const = 0;
    virtual std::string invalid_region_error() const = 0;
};
