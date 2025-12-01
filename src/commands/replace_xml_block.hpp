#pragma once
#include "command_base.hpp"
#include "commands/symbol_utils.h"
#include "commands/text_utils.h"
#include <stdexcept>
#include <cctype>
#include <string>

class ReplaceXmlBlockCommand : public Command
{
public:
    ReplaceXmlBlockCommand()
        : Command("replace-xml-block")
    {
    }

    void parse(const nos::trent &tr) override
    {
        using namespace command_parse;
        filepath_ = get_scalar(tr, "path");
        if (filepath_.empty())
            throw std::runtime_error(
                "replace-xml-block: missing 'path'");

        indent_from_marker_ = parse_indent_from_options(tr, true);

        const auto &dict = tr.as_dict();
        auto it_comment = dict.find("comment");
        if (it_comment != dict.end() && !it_comment->second.is_nil())
            comment_ = it_comment->second.as_string();

        std::string marker_text = get_scalar(tr, "marker");
        if (marker_text.empty())
            throw std::runtime_error(
                "replace-xml-block: 'marker' cannot be empty");
        marker_ = split_scalar_lines(marker_text);

        std::string payload_text = get_scalar(tr, "payload");
        if (payload_text.empty())
            throw std::runtime_error(
                "replace-xml-block: 'payload' is required and cannot be empty");
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
                "replace-xml-block: empty marker after parse");

        PatchLanguage lang = detect_language(language_);
        auto matches = find_marker_matches(lines, marker_, lang);
        if (matches.empty())
        {
            throw std::runtime_error(
                "replace-xml-block: marker not found in file: " + filepath_);
        }

        int idx =
            find_best_marker_match(lines, lang, before_, after_, matches);
        if (idx < 0 || idx >= static_cast<int>(matches.size()))
            throw std::runtime_error(
                "replace-xml-block: internal error choosing marker match");

        const MarkerMatch &mm = matches[static_cast<std::size_t>(idx)];

        int open_line = -1;
        int open_col = -1;
        std::string tag_name;

        const int nlines = static_cast<int>(lines.size());

        for (int li = mm.begin; li <= mm.end && li < nlines; ++li)
        {
            if (li < 0)
                continue;
            const std::string &ln =
                lines[static_cast<std::size_t>(li)];
            for (std::size_t col = 0; col < ln.size(); ++col)
            {
                char c = ln[col];
                if (c != '<')
                    continue;

                std::size_t pos = col + 1;
                while (pos < ln.size() &&
                       (ln[pos] == ' ' || ln[pos] == '\t' ||
                        ln[pos] == '\r' || ln[pos] == '\n'))
                {
                    ++pos;
                }
                if (pos >= ln.size())
                    continue;
                if (ln[pos] == '/' || ln[pos] == '!' || ln[pos] == '?')
                    continue;

                std::size_t name_start = pos;
                while (pos < ln.size() &&
                       (std::isalnum(static_cast<unsigned char>(ln[pos])) ||
                        ln[pos] == '_' || ln[pos] == '-' ||
                        ln[pos] == ':' || ln[pos] == '.'))
                {
                    ++pos;
                }
                if (name_start == pos)
                    continue;

                tag_name = ln.substr(name_start, pos - name_start);
                open_line = li;
                open_col = static_cast<int>(col);
                break;
            }
            if (open_line >= 0)
                break;
        }

        if (open_line < 0)
        {
            for (int li = mm.end + 1; li < nlines && open_line < 0; ++li)
            {
                if (li < 0)
                    continue;
                const std::string &ln =
                    lines[static_cast<std::size_t>(li)];
                for (std::size_t col = 0; col < ln.size(); ++col)
                {
                    char c = ln[col];
                    if (c != '<')
                        continue;

                    std::size_t pos = col + 1;
                    while (pos < ln.size() &&
                           (ln[pos] == ' ' || ln[pos] == '\t' ||
                            ln[pos] == '\r' || ln[pos] == '\n'))
                    {
                        ++pos;
                    }
                    if (pos >= ln.size())
                        continue;
                    if (ln[pos] == '/' || ln[pos] == '!' || ln[pos] == '?')
                        continue;

                    std::size_t name_start = pos;
                    while (pos < ln.size() &&
                           (std::isalnum(static_cast<unsigned char>(ln[pos])) ||
                            ln[pos] == '_' || ln[pos] == '-' ||
                            ln[pos] == ':' || ln[pos] == '.'))
                    {
                        ++pos;
                    }
                    if (name_start == pos)
                        continue;

                    tag_name = ln.substr(name_start, pos - name_start);
                    open_line = li;
                    open_col = static_cast<int>(col);
                    break;
                }
            }
        }

        if (open_line < 0 || tag_name.empty())
        {
            throw std::runtime_error(
                "replace-xml-block: opening '<tag>' not found after marker for file: " +
                filepath_);
        }

        int close_line = -1;

        bool in_comment = false;
        bool in_cdata = false;
        bool in_pi = false;

        int depth = 0;
        bool started = false;

        for (int li = open_line; li < nlines; ++li)
        {
            const std::string &ln =
                lines[static_cast<std::size_t>(li)];
            std::size_t start_col =
                li == open_line
                    ? static_cast<std::size_t>(open_col)
                    : 0;
            if (start_col > ln.size())
                start_col = ln.size();

            for (std::size_t col = start_col; col < ln.size(); ++col)
            {
                char c = ln[col];
                char next1 =
                    (col + 1 < ln.size()) ? ln[col + 1] : '\0';
                char next2 =
                    (col + 2 < ln.size()) ? ln[col + 2] : '\0';
                char next3 =
                    (col + 3 < ln.size()) ? ln[col + 3] : '\0';

                if (in_comment)
                {
                    if (c == '-' && next1 == '-' && next2 == '>')
                    {
                        in_comment = false;
                        col += 2;
                    }
                    continue;
                }
                if (in_cdata)
                {
                    if (c == ']' && next1 == ']' && next2 == '>')
                    {
                        in_cdata = false;
                        col += 2;
                    }
                    continue;
                }
                if (in_pi)
                {
                    if (c == '?' && next1 == '>')
                    {
                        in_pi = false;
                        ++col;
                    }
                    continue;
                }

                if (c != '<')
                    continue;

                if (next1 == '!' && next2 == '-' && next3 == '-')
                {
                    in_comment = true;
                    col += 3;
                    continue;
                }

                if (next1 == '!' && next2 == '[')
                {
                    if (col + 8 < ln.size() &&
                        ln[col + 2] == 'C' &&
                        ln[col + 3] == 'D' &&
                        ln[col + 4] == 'A' &&
                        ln[col + 5] == 'T' &&
                        ln[col + 6] == 'A' &&
                        ln[col + 7] == '[')
                    {
                        in_cdata = true;
                        col += 7;
                        continue;
                    }
                }

                if (next1 == '?')
                {
                    in_pi = true;
                    ++col;
                    continue;
                }

                std::size_t name_pos = col + 1;
                while (name_pos < ln.size() &&
                       (ln[name_pos] == ' ' || ln[name_pos] == '\t' ||
                        ln[name_pos] == '\r' || ln[name_pos] == '\n'))
                {
                    ++name_pos;
                }

                bool is_closing = false;
                if (name_pos < ln.size() && ln[name_pos] == '/')
                {
                    is_closing = true;
                    ++name_pos;
                }

                while (name_pos < ln.size() &&
                       (ln[name_pos] == ' ' || ln[name_pos] == '\t' ||
                        ln[name_pos] == '\r' || ln[name_pos] == '\n'))
                {
                    ++name_pos;
                }

                std::size_t name_start = name_pos;
                while (name_pos < ln.size() &&
                       (std::isalnum(
                            static_cast<unsigned char>(ln[name_pos])) ||
                        ln[name_pos] == '_' || ln[name_pos] == '-' ||
                        ln[name_pos] == ':' || ln[name_pos] == '.'))
                {
                    ++name_pos;
                }

                if (name_start == name_pos)
                    continue;

                std::string name =
                    ln.substr(name_start, name_pos - name_start);
                if (name != tag_name)
                    continue;

                bool self_closing = false;
                bool in_attr_quote = false;
                char quote_ch = '\0';

                for (std::size_t j = name_pos; j < ln.size(); ++j)
                {
                    char d = ln[j];
                    if (!in_attr_quote && (d == '"' || d == '\''))
                    {
                        in_attr_quote = true;
                        quote_ch = d;
                    }
                    else if (in_attr_quote && d == quote_ch)
                    {
                        in_attr_quote = false;
                    }

                    if (!in_attr_quote && d == '>')
                    {
                        std::size_t k = j;
                        while (k > name_pos &&
                               (ln[k - 1] == ' ' || ln[k - 1] == '\t' ||
                                ln[k - 1] == '\r' || ln[k - 1] == '\n'))
                        {
                            --k;
                        }
                        if (k > name_pos && ln[k - 1] == '/')
                            self_closing = true;
                        break;
                    }
                }

                if (!is_closing)
                {
                    if (!started)
                    {
                        started = true;
                        depth = 1;
                    }
                    else
                    {
                        ++depth;
                    }
                    if (self_closing)
                    {
                        --depth;
                        if (started && depth == 0)
                        {
                            close_line = li;
                            break;
                        }
                    }
                }
                else
                {
                    if (!started)
                        continue;
                    --depth;
                    if (depth == 0)
                    {
                        close_line = li;
                        break;
                    }
                }
            }

            if (close_line >= 0)
                break;
        }

        if (close_line < 0)
        {
            throw std::runtime_error(
                "replace-xml-block: matching closing tag '</" + tag_name +
                ">' not found for file: " + filepath_);
        }

        std::size_t region_start =
            static_cast<std::size_t>(mm.begin);
        std::size_t region_end =
            static_cast<std::size_t>(close_line);
        if (region_start > region_end ||
            region_end >= lines.size())
        {
            throw std::runtime_error(
                "replace-xml-block: internal region calculation error for file: " +
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
