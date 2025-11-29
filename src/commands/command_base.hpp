#pragma once

#include "command.h"
#include "command_parse_helpers.h"
#include "commands/text_utils.h"
#include "commands/symbol_utils.h"
#include "languages/symbols.h"
#include "apply.h"
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

        if (chunk_verbose_logging_enabled())
            debug_file_text_ = join_lines(lines);
        else
            debug_file_text_.clear();

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

        if (!chunk_verbose_logging_enabled())
            return;
        if (debug_file_text_.empty() || marker_.empty())
            return;

        std::string marker_text = join_lines(marker_);
        if (marker_text.empty())
            return;

        const std::string &file_text = debug_file_text_;
        std::size_t best_pos = std::string::npos;
        std::size_t best_len = 0;
        if (!file_text.empty())
        {
            for (std::size_t i = 0; i < file_text.size(); ++i)
            {
                std::size_t j = 0;
                while (i + j < file_text.size() && j < marker_text.size() &&
                       file_text[i + j] == marker_text[j])
                {
                    ++j;
                }
                if (j > best_len)
                {
                    best_len = j;
                    best_pos = i;
                    if (best_len == marker_text.size())
                        break;
                }
            }
        }
        if (best_pos == std::string::npos)
            return;

        std::size_t mismatch_file_pos = best_pos + best_len;
        std::size_t mismatch_marker_pos = best_len;

        os << "\nverbose marker diagnostics:\n";
        os << "  best marker prefix match length: " << best_len
           << " of " << marker_text.size() << " bytes\n";

        if (best_len == marker_text.size())
        {
            os << "  note: full marker matched; most likely error is in marker search logic\n";
            return;
        }

        std::size_t line = 1;
        std::size_t column = 1;
        for (std::size_t i = 0; i < mismatch_file_pos && i < file_text.size(); ++i)
        {
            if (file_text[i] == '\n')
            {
                ++line;
                column = 1;
            }
            else
            {
                ++column;
            }
        }
        os << "  file position: offset " << mismatch_file_pos
           << ", line " << line << ", column " << column << "\n";

        auto hex_of = [](unsigned char c) -> std::string {
            char buf[3];
            unsigned char hi = static_cast<unsigned char>((c >> 4) & 0x0F);
            unsigned char lo = static_cast<unsigned char>(c & 0x0F);
            buf[0] = static_cast<char>(hi < 10 ? ('0' + hi) : ('A' + (hi - 10)));
            buf[1] = static_cast<char>(lo < 10 ? ('0' + lo) : ('A' + (lo - 10)));
            buf[2] = '\0';
            return std::string(buf, 2);
        };

        if (mismatch_file_pos < file_text.size())
        {
            unsigned char fc =
                static_cast<unsigned char>(file_text[mismatch_file_pos]);
            os << "  file mismatch byte (hex): 0x" << hex_of(fc) << "\n";
        }
        else
        {
            os << "  note: mismatch is at end of file\n";
        }

        if (mismatch_marker_pos < marker_text.size())
        {
            unsigned char mc =
                static_cast<unsigned char>(marker_text[mismatch_marker_pos]);
            os << "  marker byte (hex):        0x" << hex_of(mc) << "\n";
        }

        const std::size_t max_tail = 32;
        if (mismatch_file_pos < file_text.size())
        {
            std::size_t available = file_text.size() - mismatch_file_pos;
            std::size_t len = available < max_tail ? available : max_tail;
            os << "  file tail (hex):";
            for (std::size_t i = 0; i < len; ++i)
            {
                unsigned char c =
                    static_cast<unsigned char>(file_text[mismatch_file_pos + i]);
                os << " " << hex_of(c);
            }
            os << "\n";
        }

        auto print_context = [&os](const char *label,
                                   const std::string &text,
                                   std::size_t pos) {
            if (text.empty() || pos >= text.size())
                return;
            std::size_t start = pos;
            while (start > 0 && text[start - 1] != '\n' && text[start - 1] != '\r')
                --start;
            std::size_t end = pos;
            while (end < text.size() && text[end] != '\n' && text[end] != '\r')
                ++end;
            std::string line_str = text.substr(start, end - start);
            os << "  " << label << ":\n";
            os << "    " << line_str << "\n";
            os << "    ";
            std::size_t caret_pos = pos - start;
            for (std::size_t i = 0; i < caret_pos && i < line_str.size(); ++i)
            {
                char c = line_str[i];
                os << (c == '\t' ? '\t' : ' ');
            }
            os << "^\n";
        };

        print_context("marker context around mismatch", marker_text, mismatch_marker_pos);
        print_context("file context around mismatch", file_text, mismatch_file_pos);
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

    std::string debug_file_text_;
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
