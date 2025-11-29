#pragma once
#include "command_base.hpp"
#include "commands/symbol_utils.h"
#include "commands/text_utils.h"
#include <stdexcept>

class ReplaceCStyleBlockCommand : public Command
{
public:
    ReplaceCStyleBlockCommand()
        : Command("replace-c-style-block")
    {
    }

    void parse(const nos::trent &tr) override
    {
        using namespace command_parse;

        filepath_ = get_scalar(tr, "path");
        if (filepath_.empty())
            throw std::runtime_error(
                "replace-c-style-block: missing 'path'");

        indent_from_marker_ = parse_indent_from_options(tr, true);

        const auto &dict = tr.as_dict();
        auto it_comment = dict.find("comment");
        if (it_comment != dict.end() && !it_comment->second.is_nil())
            comment_ = it_comment->second.as_string();

        std::string marker_text = get_scalar(tr, "marker");
        if (marker_text.empty())
            throw std::runtime_error(
                "replace-c-style-block: 'marker' cannot be empty");
        marker_ = split_scalar_lines(marker_text);

        std::string payload_text = get_scalar(tr, "payload");
        if (payload_text.empty())
            throw std::runtime_error(
                "replace-c-style-block: 'payload' is required and cannot be empty");
        payload_ = split_scalar_lines(payload_text);

        auto it_before = dict.find("before");
        if (it_before != dict.end() && !it_before->second.is_nil())
            before_ = split_scalar_lines(it_before->second.as_string());

        auto it_after = dict.find("after");
        if (it_after != dict.end() && !it_after->second.is_nil())
            after_ = split_scalar_lines(it_after->second.as_string());
    }

    void execute(std::vector<std::string> &lines) override
    {
        using namespace text_utils;

        if (marker_.empty())
            throw std::runtime_error(
                "replace-c-style-block: empty marker after parse");

        PatchLanguage lang = detect_language(language_);
        auto matches = find_marker_matches(lines, marker_, lang);
        if (matches.empty())
        {
            throw std::runtime_error(
                "replace-c-style-block: marker not found in file: " + filepath_);
        }

        int idx =
            find_best_marker_match(lines, lang, before_, after_, matches);
        if (idx < 0 ||
            idx >= static_cast<int>(matches.size()))
            throw std::runtime_error(
                "replace-c-style-block: internal error choosing marker match");

        const MarkerMatch &mm = matches[static_cast<std::size_t>(idx)];

        int open_line = -1;
        int open_col = -1;
        for (int li = mm.begin; li <= mm.end; ++li)
        {
            if (li < 0 ||
                li >= static_cast<int>(lines.size()))
                continue;

            const std::string &ln =
                lines[static_cast<std::size_t>(li)];
            for (std::size_t col = 0; col < ln.size(); ++col)
            {
                if (ln[col] == '{')
                {
                    open_line = li;
                    open_col = static_cast<int>(col);
                }
            }
        }

        if (open_line < 0)
        {
            throw std::runtime_error(
                "replace-c-style-block: opening brace '{' not found inside marker for file: " +
                filepath_);
        }

        int close_line = -1;
        int close_col = -1;

        bool in_block_comment = false;
        bool in_single_quote = false;
        bool in_double_quote = false;
        bool escape = false;

        int depth = 0;
        bool started = false;

        const int nlines =
            static_cast<int>(lines.size());
        for (int li = open_line; li < nlines; ++li)
        {
            const std::string &ln =
                lines[static_cast<std::size_t>(li)];
            bool in_line_comment = false;

            std::size_t start_col =
                li == open_line
                    ? static_cast<std::size_t>(open_col)
                    : 0;
            if (start_col > ln.size())
                start_col = ln.size();

            for (std::size_t col = start_col; col < ln.size(); ++col)
            {
                char c = ln[col];
                char next =
                    (col + 1 < ln.size()) ? ln[col + 1] : '\0';

                if (in_line_comment)
                    break;

                if (in_block_comment)
                {
                    if (c == '*' && next == '/')
                    {
                        in_block_comment = false;
                        ++col;
                    }
                    continue;
                }

                if (in_single_quote)
                {
                    if (escape)
                    {
                        escape = false;
                        continue;
                    }
                    if (c == '\\')
                    {
                        escape = true;
                        continue;
                    }
                    if (c == '\'')
                    {
                        in_single_quote = false;
                    }
                    continue;
                }

                if (in_double_quote)
                {
                    if (escape)
                    {
                        escape = false;
                        continue;
                    }
                    if (c == '\\')
                    {
                        escape = true;
                        continue;
                    }
                    if (c == '"')
                    {
                        in_double_quote = false;
                    }
                    continue;
                }

                if (c == '/' && next == '/')
                {
                    in_line_comment = true;
                    break;
                }
                if (c == '/' && next == '*')
                {
                    in_block_comment = true;
                    ++col;
                    continue;
                }
                if (c == '\'')
                {
                    in_single_quote = true;
                    continue;
                }
                if (c == '"')
                {
                    in_double_quote = true;
                    continue;
                }

                if (c == '{')
                {
                    ++depth;
                    started = true;
                }
                else if (c == '}')
                {
                    if (started)
                    {
                        --depth;
                        if (depth == 0)
                        {
                            close_line = li;
                            close_col = static_cast<int>(col);
                            (void)close_col;
                            break;
                        }
                    }
                }
            }

            if (close_line >= 0)
                break;
        }

        if (close_line < 0)
        {
            throw std::runtime_error(
                "replace-c-style-block: matching closing brace '}' not found for file: " +
                filepath_);
        }

        std::size_t region_start =
            static_cast<std::size_t>(mm.begin);
        std::size_t region_end =
            static_cast<std::size_t>(close_line);

        if (region_start > region_end ||
            region_end >= lines.size())
        {
            throw std::runtime_error(
                "replace-c-style-block: internal region calculation error for file: " +
                filepath_);
        }

        std::vector<std::string> adjusted_payload;
        if (indent_from_marker_)
        {
            std::string prefix =
                extract_indent_prefix(lines, mm.begin);
            adjusted_payload =
                apply_indent_prefix(payload_, prefix, true);
        }
        else
        {
            adjusted_payload = payload_;
        }

        auto it_begin = lines.begin() +
                        static_cast<std::ptrdiff_t>(region_start);
        auto it_end = lines.begin() +
                      static_cast<std::ptrdiff_t>(region_end + 1);
        lines.erase(it_begin, it_end);
        lines.insert(lines.begin() +
                         static_cast<std::ptrdiff_t>(region_start),
                     adjusted_payload.begin(), adjusted_payload.end());
    }

private:
    bool indent_from_marker_ = true;
    std::vector<std::string> marker_;
    std::vector<std::string> before_;
    std::vector<std::string> after_;
    std::vector<std::string> payload_;
};
