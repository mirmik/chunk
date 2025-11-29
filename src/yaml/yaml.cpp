#include <cctype>
#include <cstdlib>
#include <fstream>
#include <yaml/yaml.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <limits>

namespace nos
{
    namespace yaml
    {
        parse_error::parse_error(size_t line,
                                 size_t column,
                                 const std::string &msg)
            : std::runtime_error([&]() {
                  std::ostringstream os;
                  os << "yaml: line " << line;
                  if (column != 0)
                      os << ", column " << column;
                  os << ": " << msg;
                  return os.str();
              }()),
              line_(line),
              column_(column)
        {
        }

        namespace detail
        {
            struct line
            {
                int indent = 0;
                std::string raw;
                std::string no_comment;
                std::string trimmed;
                size_t number = 0;
            };

            inline std::string ltrim(const std::string &s)
            {
                size_t i = 0;
                while (i < s.size() &&
                       (s[i] == ' ' || s[i] == '\t' || s[i] == '\r'))
                    ++i;
                return s.substr(i);
            }

            inline std::string rtrim(const std::string &s)
            {
                if (s.empty())
                    return s;
                size_t end = s.size() - 1;
                while (end < s.size() &&
                       (s[end] == ' ' || s[end] == '\t' || s[end] == '\r'))
                {
                    if (end == 0)
                        return "";
                    --end;
                }
                return s.substr(0, end + 1);
            }

            inline std::string trim(const std::string &s)
            {
                return rtrim(ltrim(s));
            }

            inline size_t compute_column(const std::string &text, size_t pos)
            {
                size_t col = 1;
                for (size_t i = 0; i < pos && i < text.size(); ++i)
                {
                    col += (text[i] == '\t') ? 4 : 1;
                }
                return col;
            }

            inline size_t offset_for_indent(const std::string &text,
                                            int indent)
            {
                int col = 0;
                size_t i = 0;
                while (i < text.size() && col < indent)
                {
                    col += (text[i] == '\t') ? 4 : 1;
                    ++i;
                }
                return i;
            }

            inline std::string strip_comment(const std::string &text)
            {
                bool in_single = false;
                bool in_double = false;
                std::string result;
                result.reserve(text.size());

                for (size_t i = 0; i < text.size(); ++i)
                {
                    char ch = text[i];

                    if (ch == '"' && !in_single)
                    {
                        in_double = !in_double;
                        result.push_back(ch);
                        continue;
                    }

                    if (ch == '\'' && !in_double)
                    {
                        if (in_single && i + 1 < text.size() && text[i + 1] == '\'')
                        {
                            result.push_back(ch);
                            result.push_back(text[i + 1]);
                            ++i;
                            continue;
                        }
                        in_single = !in_single;
                        result.push_back(ch);
                        continue;
                    }

                    if (in_double && ch == '\\' && i + 1 < text.size())
                    {
                        result.push_back(ch);
                        result.push_back(text[i + 1]);
                        ++i;
                        continue;
                    }

                    if (!in_single && !in_double && ch == '#')
                    {
                        if (i == 0 ||
                            std::isspace(static_cast<unsigned char>(text[i - 1])))
                            break;
                    }

                    result.push_back(ch);
                }

                return result;
            }

            inline bool needs_quotes(const std::string &s)
            {
                if (s.empty())
                    return true;
                for (size_t i = 0; i < s.size(); ++i)
                {
                    char c = s[i];
                    if (std::isspace(static_cast<unsigned char>(c)) ||
                        c == ':' || c == '-' || c == '#' || c == '[' ||
                        c == ']' || c == '{' || c == '}' || c == ',' ||
                        c == '\'' || c == '"' || c == '\\')
                        return true;
                }
                return false;
            }

            inline std::string escape_string(const std::string &str)
            {
                std::string out;
                out.reserve(str.size() + 4);
                out.push_back('"');
                for (unsigned char c : str)
                {
                    switch (c)
                    {
                    case '"':
                        out += "\\\"";
                        break;
                    case '\\':
                        out += "\\\\";
                        break;
                    case '\b':
                        out += "\\b";
                        break;
                    case '\f':
                        out += "\\f";
                        break;
                    case '\n':
                        out += "\\n";
                        break;
                    case '\r':
                        out += "\\r";
                        break;
                    case '\t':
                        out += "\\t";
                        break;
                    default:
                        if (c < 0x20)
                        {
                            const char hex[] = "0123456789ABCDEF";
                            out += "\\u00";
                            out.push_back(hex[(c >> 4) & 0x0F]);
                            out.push_back(hex[c & 0x0F]);
                        }
                        else
                        {
                            out.push_back(static_cast<char>(c));
                        }
                        break;
                    }
                }
                out.push_back('"');
                return out;
            }

            inline std::string scalar_to_string(const nos::trent &tr)
            {
                switch (tr.get_type())
                {
                case nos::trent::type::boolean:
                    return tr.as_bool() ? "true" : "false";
                case nos::trent::type::numer: {
                    std::ostringstream os;
                    os << tr.as_numer();
                    return os.str();
                }
                case nos::trent::type::nil:
                    return "null";
                case nos::trent::type::string: {
                    const auto &val = tr.as_string();
                    if (needs_quotes(val))
                        return escape_string(val);
                    return val;
                }
                default:
                    return "";
                }
            }

            static std::string remove_numeric_separators(
                const std::string &text)
            {
                std::string out;
                out.reserve(text.size());
                for (char c : text)
                {
                    if (c != '_')
                        out.push_back(c);
                }
                return out;
            }

            static std::string to_lower(const std::string &s)
            {
                std::string out = s;
                for (auto &ch : out)
                    ch = static_cast<char>(
                        std::tolower(static_cast<unsigned char>(ch)));
                return out;
            }

            static std::string parse_single_quoted(const std::string &text,
                                                   size_t line,
                                                   size_t column)
            {
                if (text.size() < 2 || text.back() != '\'')
                    throw nos::yaml::parse_error(
                        line, column, "unterminated single-quoted string");

                std::string result;
                for (size_t i = 1; i < text.size() - 1; ++i)
                {
                    char ch = text[i];
                    if (ch == '\'' && i + 1 < text.size() - 1 &&
                        text[i + 1] == '\'')
                    {
                        result.push_back('\'');
                        ++i;
                    }
                    else
                    {
                        result.push_back(ch);
                    }
                }
                return result;
            }

            static uint32_t hex_to_int(const std::string &hex)
            {
                uint32_t value = 0;
                for (char c : hex)
                {
                    value <<= 4;
                    if (c >= '0' && c <= '9')
                        value |= static_cast<uint32_t>(c - '0');
                    else if (c >= 'a' && c <= 'f')
                        value |= static_cast<uint32_t>(10 + c - 'a');
                    else if (c >= 'A' && c <= 'F')
                        value |= static_cast<uint32_t>(10 + c - 'A');
                    else
                        return std::numeric_limits<uint32_t>::max();
                }
                return value;
            }

            static std::string parse_double_quoted(const std::string &text,
                                                   size_t line,
                                                   size_t column)
            {
                if (text.size() < 2 || text.back() != '"')
                    throw nos::yaml::parse_error(
                        line, column, "unterminated double-quoted string");

                std::string result;
                for (size_t i = 1; i < text.size() - 1; ++i)
                {
                    char ch = text[i];
                    if (ch == '\\')
                    {
                        if (i + 1 >= text.size() - 1)
                            throw nos::yaml::parse_error(
                                line,
                                column + i,
                                "bad escape sequence");

                        char next = text[++i];
                        switch (next)
                        {
                        case 'n':
                            result.push_back('\n');
                            break;
                        case 'r':
                            result.push_back('\r');
                            break;
                        case 't':
                            result.push_back('\t');
                            break;
                        case '"':
                            result.push_back('"');
                            break;
                        case '\\':
                            result.push_back('\\');
                            break;
                        case 'b':
                            result.push_back('\b');
                            break;
                        case 'f':
                            result.push_back('\f');
                            break;
                        case '0':
                            result.push_back('\0');
                            break;
                        case 'u':
                        {
                            if (i + 4 >= text.size() - 1)
                                throw nos::yaml::parse_error(
                                    line,
                                    column + i,
                                    "incomplete unicode escape");
                            std::string hex = text.substr(i + 1, 4);
                            uint32_t code = hex_to_int(hex);
                            if (code == std::numeric_limits<uint32_t>::max())
                                throw nos::yaml::parse_error(
                                    line,
                                    column + i,
                                    "invalid unicode escape");
                            if (code <= 0x7F)
                            {
                                result.push_back(
                                    static_cast<char>(code & 0x7F));
                            }
                            else if (code <= 0x7FF)
                            {
                                result.push_back(
                                    static_cast<char>(0xC0 | (code >> 6)));
                                result.push_back(
                                    static_cast<char>(0x80 | (code & 0x3F)));
                            }
                            else
                            {
                                result.push_back(
                                    static_cast<char>(0xE0 | (code >> 12)));
                                result.push_back(
                                    static_cast<char>(0x80 |
                                                      ((code >> 6) & 0x3F)));
                                result.push_back(
                                    static_cast<char>(0x80 |
                                                      (code & 0x3F)));
                            }
                            i += 4;
                            break;
                        }
                        default:
                            result.push_back(next);
                            break;
                        }
                        continue;
                    }
                    result.push_back(ch);
                }
                return result;
            }

            static nos::trent parse_scalar_text(const std::string &text,
                                                size_t line,
                                                size_t column)
            {
                std::string trimmed = trim(text);
                if (trimmed.empty())
                    return nos::trent::nil();

                if (trimmed.front() == '"' && trimmed.back() == '"' &&
                    trimmed.size() >= 2)
                {
                    return parse_double_quoted(trimmed, line, column);
                }

                if (trimmed.front() == '\'' && trimmed.back() == '\'' &&
                    trimmed.size() >= 2)
                {
                    return parse_single_quoted(trimmed, line, column);
                }

                auto lower = to_lower(trimmed);
                if (lower == "true" || lower == "false")
                    return nos::trent(lower == "true");
                if (lower == "null" || lower == "~")
                    return nos::trent::nil();
                if (lower == ".inf" || lower == "+.inf")
                    return nos::trent(
                        std::numeric_limits<long double>::infinity());
                if (lower == "-.inf")
                    return nos::trent(
                        -std::numeric_limits<long double>::infinity());
                if (lower == ".nan")
                    return nos::trent(
                        std::numeric_limits<long double>::quiet_NaN());

                std::string numeric = remove_numeric_separators(trimmed);

                char *endptr = nullptr;
                double value = std::strtod(numeric.c_str(), &endptr);
                if (endptr && *endptr == '\0' &&
                    endptr != numeric.c_str())
                {
                    return nos::trent(static_cast<long double>(value));
                }

                return nos::trent(trimmed);
            }

            class parser
            {
                std::vector<line> lines;
                size_t index = 0;
                bool in_block_scalar = false;
                int block_indent = 0;

                static size_t find_block_indicator(const std::string &text,
                                                   size_t start_pos)
                {
                    bool in_single = false;
                    bool in_double = false;

                    for (size_t i = start_pos; i < text.size(); ++i)
                    {
                        char ch = text[i];
                        if (ch == '"' && !in_single)
                        {
                            in_double = !in_double;
                            continue;
                        }
                        if (ch == '\'' && !in_double)
                        {
                            if (in_single && i + 1 < text.size() &&
                                text[i + 1] == '\'')
                            {
                                ++i;
                                continue;
                            }
                            in_single = !in_single;
                            continue;
                        }
                        if (!in_single && !in_double && (ch == '|' || ch == '>'))
                            return i;
                        if (in_double && ch == '\\')
                            ++i;
                    }
                    return std::string::npos;
                }

                static int compute_block_indent(const std::string &text,
                                                size_t indicator_pos,
                                                int indent)
                {
                    int explicit_indent = 0;
                    size_t pos = indicator_pos + 1;

                    while (pos < text.size())
                    {
                        char ch = text[pos];
                        if (ch == '+' || ch == '-')
                        {
                            ++pos;
                            continue;
                        }
                        if (std::isdigit(static_cast<unsigned char>(ch)))
                        {
                            explicit_indent = explicit_indent * 10 + (ch - '0');
                            ++pos;
                            continue;
                        }
                        if (ch == ' ' || ch == '\t')
                        {
                            ++pos;
                            continue;
                        }
                        break;
                    }

                    if (explicit_indent > 0)
                        return indent + explicit_indent;
                    return indent + 1;
                }

                void add_line(std::string line_text, size_t line_no)
                {
                    size_t pos = 0;
                    int indent = 0;
                    while (pos < line_text.size() &&
                           (line_text[pos] == ' ' || line_text[pos] == '\t'))
                    {
                        indent += (line_text[pos] == '\t') ? 4 : 1;
                        ++pos;
                    }

                    bool is_block_content = false;
                    size_t non_ws = line_text.find_first_not_of(" \t\r");
                    if (in_block_scalar)
                    {
                        if (indent >= block_indent || non_ws == std::string::npos)
                        {
                            is_block_content = true;
                        }
                        else
                        {
                            in_block_scalar = false;
                        }
                    }

                    std::string no_comment =
                        is_block_content ? line_text : strip_comment(line_text);
                    std::string trimmed = trim(no_comment);

                    lines.push_back(
                        {indent, line_text, no_comment, trimmed, line_no});

                    if (!in_block_scalar)
                    {
                        size_t value_pos = std::string::npos;

                        size_t colon = find_unescaped_colon(no_comment);
                        if (colon != std::string::npos)
                        {
                            value_pos =
                                no_comment.find_first_not_of(" \t", colon + 1);
                        }
                        else if (non_ws != std::string::npos &&
                                 no_comment[non_ws] == '-')
                        {
                            value_pos =
                                no_comment.find_first_not_of(" \t", non_ws + 1);
                        }
                        else
                        {
                            value_pos = non_ws;
                        }

                        if (value_pos != std::string::npos)
                        {
                            size_t indicator_pos =
                                find_block_indicator(no_comment, value_pos);
                            if (indicator_pos != std::string::npos)
                            {
                                in_block_scalar = true;
                                block_indent = compute_block_indent(
                                    no_comment, indicator_pos, indent);
                            }
                        }
                    }
                }

                static size_t find_unescaped_colon(const std::string &text)
                {
                    bool in_single = false;
                    bool in_double = false;
                    for (size_t i = 0; i < text.size(); ++i)
                    {
                        char ch = text[i];
                        if (ch == '"' && !in_single)
                        {
                            in_double = !in_double;
                            continue;
                        }
                        if (ch == '\'' && !in_double)
                        {
                            if (in_single && i + 1 < text.size() &&
                                text[i + 1] == '\'')
                            {
                                ++i;
                                continue;
                            }
                            in_single = !in_single;
                            continue;
                        }
                        if (ch == ':' && !in_single && !in_double)
                        {
                            if (i + 1 == text.size() ||
                                std::isspace(static_cast<unsigned char>(
                                    text[i + 1])))
                                return i;
                        }
                    }
                    return std::string::npos;
                }

                bool is_document_marker(const line &ln) const
                {
                    return ln.trimmed == "---" || ln.trimmed == "...";
                }

                bool is_sequence_line(size_t idx, int indent) const
                {
                    if (idx >= lines.size())
                        return false;
                    const auto &ln = lines[idx];
                    if (ln.indent != indent)
                        return false;
                    size_t pos = ln.no_comment.find_first_not_of(" \t");
                    if (pos == std::string::npos)
                        return false;
                    return ln.no_comment[pos] == '-' &&
                           (pos + 1 == ln.no_comment.size() ||
                            std::isspace(static_cast<unsigned char>(
                                ln.no_comment[pos + 1])));
                }

                bool is_mapping_line(size_t idx, int indent) const
                {
                    if (idx >= lines.size())
                        return false;
                    const auto &ln = lines[idx];
                    if (ln.indent != indent)
                        return false;
                    return find_unescaped_colon(ln.no_comment) !=
                           std::string::npos;
                }

                void skip_empty_lines()
                {
                    while (index < lines.size())
                    {
                        const auto &ln = lines[index];
                        if (ln.trimmed.empty() || is_document_marker(ln))
                        {
                            ++index;
                            continue;
                        }
                        break;
                    }
                }

                nos::trent parse_block_scalar(const line &ln,
                                              size_t indicator_pos,
                                              size_t line_idx)
                {
                    char indicator = ln.no_comment[indicator_pos];
                    char chomping = 0;
                    int explicit_indent = 0;
                    size_t pos = indicator_pos + 1;

                    while (pos < ln.no_comment.size())
                    {
                        char ch = ln.no_comment[pos];
                        if (ch == '+' || ch == '-')
                        {
                            if (chomping == 0)
                                chomping = ch;
                            ++pos;
                            continue;
                        }
                        if (std::isdigit(static_cast<unsigned char>(ch)))
                        {
                            explicit_indent =
                                explicit_indent * 10 + (ch - '0');
                            ++pos;
                            continue;
                        }
                        if (ch == ' ' || ch == '\t')
                        {
                            ++pos;
                            continue;
                        }
                        break;
                    }

                    size_t content_start = line_idx + 1;
                    int content_indent = 0;
                    if (explicit_indent > 0)
                        content_indent = ln.indent + explicit_indent;

                    size_t probe = content_start;
                    if (content_indent == 0)
                    {
                        while (probe < lines.size())
                        {
                            if (lines[probe].trimmed.empty())
                            {
                                ++probe;
                                continue;
                            }
                            if (lines[probe].indent <= ln.indent)
                                break;
                            content_indent = lines[probe].indent;
                            break;
                        }
                    }

                    if (content_indent == 0)
                    {
                        index = probe;
                        return nos::trent(std::string());
                    }

                    std::vector<std::string> collected;
                    size_t idx = content_start;
                    while (idx < lines.size())
                    {
                        const auto &cur = lines[idx];
                        if (cur.indent < content_indent &&
                            !cur.trimmed.empty())
                            break;
                        if (cur.indent < content_indent &&
                            cur.trimmed.empty())
                        {
                            collected.emplace_back(std::string());
                            ++idx;
                            continue;
                        }

                        size_t offset =
                            offset_for_indent(cur.raw, content_indent);
                        if (offset > cur.raw.size())
                            offset = cur.raw.size();
                        collected.emplace_back(cur.raw.substr(offset));
                        ++idx;
                    }
                    index = idx;

                    if (chomping != '+')
                    {
                        while (!collected.empty() && collected.back().empty())
                            collected.pop_back();
                    }

                    std::string text;
                    if (indicator == '|')
                    {
                        for (size_t i = 0; i < collected.size(); ++i)
                        {
                            text += collected[i];
                            if (i + 1 < collected.size())
                                text.push_back('\n');
                        }
                    }
                    else
                    {
                        bool first = true;
                        bool previous_blank = false;
                        for (const auto &ln_text : collected)
                        {
                            bool blank = ln_text.empty();
                            if (blank)
                            {
                                text.push_back('\n');
                                previous_blank = true;
                                first = false;
                                continue;
                            }

                            if (!first && !previous_blank)
                                text.push_back(' ');
                            if (previous_blank)
                            {
                                if (!text.empty() &&
                                    text.back() != '\n')
                                    text.push_back('\n');
                            }

                            text += ln_text;
                            previous_blank = false;
                            first = false;
                        }
                    }

                    if (chomping == '+')
                    {
                        text.push_back('\n');
                    }
                    else if (chomping == 0)
                    {
                        if (!text.empty() && text.back() != '\n')
                            text.push_back('\n');
                    }

                    return nos::trent(text);
                }

                class flow_parser
                {
                    const std::string &src;
                    size_t pos = 0;
                    size_t line = 0;
                    size_t column = 0;
                    size_t start_line = 0;
                    size_t start_col = 0;

                    char peek() const
                    {
                        return pos < src.size() ? src[pos] : '\0';
                    }

                    bool eof() const
                    {
                        return pos >= src.size();
                    }

                    char get()
                    {
                        char c = peek();
                        ++pos;
                        if (c == '\n')
                        {
                            ++line;
                            column = 1;
                        }
                        else
                        {
                            ++column;
                        }
                        return c;
                    }

                    [[noreturn]] void fail(const std::string &msg) const
                    {
                        throw nos::yaml::parse_error(line, column, msg);
                    }

                    void skip_ws_and_comments()
                    {
                        while (!eof())
                        {
                            char c = peek();
                            if (c == ' ' || c == '\t' || c == '\r' ||
                                c == '\n')
                            {
                                get();
                                continue;
                            }
                            break;
                        }
                    }

                    nos::trent parse_plain_scalar()
                    {
                        size_t start = pos;
                        while (!eof())
                        {
                            char c = peek();
                            if (c == ',' || c == ']' || c == '}' || c == '\n')
                                break;
                            if (c == ':')
                            {
                                size_t next = pos + 1;
                                if (next < src.size() &&
                                    !std::isspace(static_cast<unsigned char>(
                                        src[next])))
                                {
                                    // part of scalar
                                }
                                else
                                {
                                    break;
                                }
                            }
                            get();
                        }
                        std::string token = src.substr(start, pos - start);
                        return parse_scalar_text(token, line, column);
                    }

                    nos::trent parse_double()
                    {
                        size_t begin = pos;
                        get(); // consume "
                        while (!eof())
                        {
                            char c = get();
                            if (c == '"')
                                break;
                            if (c == '\\' && !eof())
                                get();
                        }
                        size_t end = pos;
                        std::string token = src.substr(begin, end - begin);
                        if (token.size() < 2 || token.back() != '"')
                            fail("unterminated string");
                        return parse_double_quoted(
                            token, start_line, start_col);
                    }

                    nos::trent parse_single()
                    {
                        size_t begin = pos;
                        get(); // consume '
                        while (!eof())
                        {
                            char c = get();
                            if (c == '\'')
                            {
                                if (peek() == '\'')
                                    get();
                                else
                                    break;
                            }
                        }
                        size_t end = pos;
                        std::string token = src.substr(begin, end - begin);
                        if (token.size() < 2 || token.back() != '\'')
                            fail("unterminated string");
                        return parse_single_quoted(
                            token, start_line, start_col);
                    }

                    nos::trent parse_array()
                    {
                        nos::trent arr;
                        arr.as_list();
                        get(); // [
                        skip_ws_and_comments();
                        if (peek() == ']')
                        {
                            get();
                            return arr;
                        }
                        while (!eof())
                        {
                            arr.as_list().push_back(parse_value());
                            skip_ws_and_comments();
                            char c = peek();
                            if (c == ',')
                            {
                                get();
                                skip_ws_and_comments();
                                continue;
                            }
                            if (c == ']')
                            {
                                get();
                                break;
                            }
                            fail("expected ',' or ']'");
                        }
                        return arr;
                    }

                    nos::trent parse_object()
                    {
                        nos::trent obj;
                        obj.as_dict();
                        get(); // {
                        skip_ws_and_comments();
                        if (peek() == '}')
                        {
                            get();
                            return obj;
                        }
                        while (!eof())
                        {
                            nos::trent key = parse_value();
                            if (!key.is_string())
                                fail("flow map keys must be strings");
                            skip_ws_and_comments();
                            if (peek() != ':')
                                fail("expected ':' in flow map");
                            get();
                            skip_ws_and_comments();
                            obj[key.as_string()] = parse_value();
                            skip_ws_and_comments();
                            char c = peek();
                            if (c == ',')
                            {
                                get();
                                skip_ws_and_comments();
                                continue;
                            }
                            if (c == '}')
                            {
                                get();
                                break;
                            }
                            fail("expected ',' or '}' in flow map");
                        }
                        return obj;
                    }

                public:
                    flow_parser(const std::string &s,
                                size_t base_line,
                                size_t base_col)
                        : src(s),
                          pos(0),
                          line(base_line),
                          column(base_col),
                          start_line(base_line),
                          start_col(base_col)
                    {
                    }

                    nos::trent parse_value()
                    {
                        skip_ws_and_comments();
                        char c = peek();
                        if (c == '[')
                            return parse_array();
                        if (c == '{')
                            return parse_object();
                        if (c == '"')
                            return parse_double();
                        if (c == '\'')
                            return parse_single();
                        return parse_plain_scalar();
                    }

                    void expect_end()
                    {
                        skip_ws_and_comments();
                        if (!eof())
                            fail("unexpected content in flow value");
                    }
                };

                std::string gather_flow_text(size_t start_idx,
                                             size_t start_pos)
                {
                    size_t idx = start_idx;
                    std::vector<char> stack;

                    bool in_single = false;
                    bool in_double = false;
                    std::string buffer;

                    while (idx < lines.size())
                    {
                        const auto &ln = lines[idx];
                        size_t pos = (idx == start_idx) ? start_pos : 0;
                        for (; pos < ln.no_comment.size(); ++pos)
                        {
                            char ch = ln.no_comment[pos];
                            buffer.push_back(ch);

                            if (!in_single && !in_double)
                            {
                                if (ch == '[')
                                    stack.push_back(']');
                                else if (ch == '{')
                                    stack.push_back('}');
                                else if (ch == ']' || ch == '}')
                                {
                                    if (stack.empty() ||
                                        stack.back() != ch)
                                        throw nos::yaml::parse_error(
                                            ln.number,
                                            compute_column(ln.no_comment,
                                                           pos + 1),
                                            "unmatched closing bracket");
                                    stack.pop_back();
                                    if (stack.empty())
                                    {
                                        std::string tail =
                                            trim(ln.no_comment.substr(
                                                pos + 1));
                                        if (!tail.empty())
                                            throw nos::yaml::parse_error(
                                                ln.number,
                                                compute_column(
                                                    ln.no_comment,
                                                    pos + 2),
                                                "unexpected text after "
                                                "flow collection");
                                        index = idx + 1;
                                        return buffer;
                                    }
                                }
                                else if (ch == '#')
                                {
                                    break;
                                }
                            }

                            if (ch == '"' && !in_single)
                                in_double = !in_double;
                            else if (ch == '\'' && !in_double)
                            {
                                if (in_single && pos + 1 < ln.no_comment.size() &&
                                    ln.no_comment[pos + 1] == '\'')
                                {
                                    buffer.push_back(
                                        ln.no_comment[pos + 1]);
                                    ++pos;
                                }
                                else
                                {
                                    in_single = !in_single;
                                }
                            }
                            else if (ch == '\\' && in_double &&
                                     pos + 1 < ln.no_comment.size())
                            {
                                buffer.push_back(ln.no_comment[pos + 1]);
                                ++pos;
                            }
                        }
                        buffer.push_back('\n');
                        ++idx;
                    }

                    throw nos::yaml::parse_error(
                        lines[start_idx].number,
                        compute_column(lines[start_idx].no_comment,
                                       start_pos + 1),
                        "unterminated flow collection");
                }

                nos::trent parse_flow_collection(const line &ln,
                                                 size_t value_pos,
                                                 size_t start_idx)
                {
                    size_t start_column =
                        compute_column(ln.no_comment, value_pos + 1);
                    if (value_pos >= ln.no_comment.size())
                        throw nos::yaml::parse_error(
                            ln.number, start_column, "invalid flow start");
                    char opener = ln.no_comment[value_pos];
                    if (opener != '[' && opener != '{')
                        throw nos::yaml::parse_error(
                            ln.number,
                            start_column,
                            "flow collection must start with '[' or '{'");
                    std::string flow =
                        gather_flow_text(start_idx, value_pos);
                    flow_parser fp(flow, ln.number, start_column);
                    nos::trent val = fp.parse_value();
                    fp.expect_end();
                    return val;
                }

                nos::trent parse_value(const line &ln,
                                       size_t value_pos,
                                       int indent,
                                       size_t line_idx)
                {
                    std::string value_text =
                        trim(ln.no_comment.substr(value_pos));
                    size_t column =
                        compute_column(ln.no_comment, value_pos + 1);

                    if (value_text.empty())
                    {
                        skip_empty_lines();
                        if (index < lines.size() &&
                            lines[index].indent > indent)
                        {
                            return parse_block(lines[index].indent);
                        }
                        return nos::trent::nil();
                    }

                    char first = value_text.front();
                    if (first == '|' || first == '>')
                    {
                        return parse_block_scalar(ln, value_pos, line_idx);
                    }
                    if (first == '[' || first == '{')
                    {
                        return parse_flow_collection(ln, value_pos, line_idx);
                    }
                    return parse_scalar_text(value_text, ln.number, column);
                }

                nos::trent parse_block(int indent)
                {
                    skip_empty_lines();
                    if (index >= lines.size())
                        return nos::trent();

                    if (lines[index].indent < indent)
                        throw nos::yaml::parse_error(
                            lines[index].number,
                            1,
                            "invalid indentation");

                    if (is_sequence_line(index, lines[index].indent))
                        return parse_sequence(lines[index].indent);

                    if (is_mapping_line(index, lines[index].indent))
                        return parse_mapping(lines[index].indent);

                    size_t current_index = index;
                    const auto &ln = lines[index];
                    ++index;

                    if (!ln.trimmed.empty() &&
                        (ln.trimmed[0] == '|' || ln.trimmed[0] == '>'))
                    {
                        return parse_block_scalar(
                            ln,
                            ln.no_comment.find_first_not_of(" \t"),
                            current_index);
                    }

                    if (!ln.trimmed.empty() &&
                        (ln.trimmed[0] == '[' || ln.trimmed[0] == '{'))
                    {
                        size_t pos =
                            ln.no_comment.find_first_not_of(" \t");
                        return parse_flow_collection(
                            ln, pos == std::string::npos ? 0 : pos, current_index);
                    }

                    return parse_scalar_text(ln.trimmed,
                                             ln.number,
                                             compute_column(ln.no_comment,
                                                            1));
                }

                nos::trent parse_sequence(int indent)
                {
                    nos::trent arr;
                    arr.as_list();
                    while (index < lines.size())
                    {
                        const auto &ln = lines[index];

                        // Пустые строки и строки, состоящие только из пробелов/комментариев,
                        // внутри последовательности пропускаем.
                        if (ln.trimmed.empty())
                        {
                            ++index;
                            continue;
                        }

                        if (!is_sequence_line(index, indent))
                            break;

                        size_t dash_pos =
                            ln.no_comment.find_first_not_of(" \t");
                        size_t value_pos =
                            ln.no_comment.find_first_not_of(
                                " \t", dash_pos + 1);

                        std::string rest;
                        if (value_pos != std::string::npos)
                            rest = trim(ln.no_comment.substr(value_pos));

                        size_t current_index = index;
                        nos::trent element;
                        bool element_initialized = false;

                        if (!rest.empty())
                        {
                            size_t colon = find_unescaped_colon(rest);
                            if (colon != std::string::npos)
                            {
                                element.init(nos::trent::type::dict);
                                std::string key = trim(rest.substr(0, colon));
                                size_t value_pos_in_line =
                                    ln.no_comment.find_first_not_of(
                                        " \t", value_pos + colon + 1);
                                std::string value_text =
                                    trim(rest.substr(colon + 1));
                                if (key.empty())
                                    throw nos::yaml::parse_error(
                                        ln.number,
                                        compute_column(ln.no_comment,
                                                       value_pos + colon + 1),
                                        "empty key in sequence mapping");
                                if (value_text.empty() ||
                                    value_pos_in_line == std::string::npos)
                                {
                                    element[key] = nos::trent::nil();
                                }
                                else
                                {
                                    element[key] = parse_value(
                                        ln,
                                        value_pos_in_line,
                                        indent,
                                        current_index);
                                }
                                element_initialized = true;
                            }
                            else
                            {
                                element = parse_value(
                                    ln, value_pos, indent, current_index);
                                element_initialized = true;
                            }
                        }

                        if (rest.empty())
                        {
                            index = current_index + 1;
                        }
                        else
                        {
                            if (index < current_index + 1)
                                index = current_index + 1;
                        }

                        if (index < lines.size() &&
                            lines[index].indent > indent)
                        {
                            nos::trent nested =
                                parse_block(lines[index].indent);
                            if (!element_initialized || element.is_nil())
                            {
                                element = nested;
                                element_initialized = true;
                            }
                            else if (element.is_dict() && nested.is_dict())
                            {
                                for (const auto &kv :
                                     nested.unsafe_dict_const())
                                {
                                    element[kv.first] = kv.second;
                                }
                            }
                            else if (element.is_list() && nested.is_list())
                            {
                                for (const auto &item :
                                     nested.unsafe_list_const())
                                {
                                    element.as_list().push_back(item);
                                }
                            }
                            else
                            {
                                element = nested;
                            }
                        }

                        if (!element_initialized)
                            element = nos::trent::nil();

                        arr.as_list().push_back(element);
                    }

                    return arr;
                }

                nos::trent parse_mapping(int indent)
                {
                    nos::trent obj;
                    obj.as_dict();
                    while (index < lines.size())
                    {
                        const auto &ln = lines[index];

                        // Пустые строки и строки-комментарии внутри mapping пропускаем.
                        if (ln.trimmed.empty())
                        {
                            ++index;
                            continue;
                        }

                        if (ln.indent != indent || !is_mapping_line(index, indent))
                            break;

                        size_t colon = find_unescaped_colon(ln.no_comment);
                        std::string key = trim(ln.no_comment.substr(0, colon));
                        size_t value_pos = ln.no_comment.find_first_not_of(
                            " \t", colon + 1);
                        std::string value_text =
                            value_pos == std::string::npos
                                ? std::string()
                                : trim(ln.no_comment.substr(colon + 1));

                        size_t current_index = index;

                        if (key.empty())
                            throw nos::yaml::parse_error(
                                ln.number,
                                compute_column(ln.no_comment, 1),
                                "empty mapping key");

                        if (!value_text.empty())
                        {
                            obj[key] =
                                parse_value(ln, value_pos, indent, current_index);
                            if (index < current_index + 1)
                                index = current_index + 1;
                            continue;
                        }

                        index = current_index + 1;

                        if (index < lines.size() &&
                            lines[index].indent > indent)
                        {
                            obj[key] = parse_block(lines[index].indent);
                        }
                        else
                        {
                            obj[key] = nos::trent::nil();
                        }
                    }

                    return obj;
                }

            public:
                explicit parser(const std::string &text)
                {
                    std::string current;
                    current.reserve(64);

                    size_t line_no = 1;
                    for (char ch : text)
                    {
                        if (ch == '\r')
                            continue;
                        if (ch == '\n')
                        {
                            add_line(current, line_no++);
                            current.clear();
                        }
                        else
                        {
                            current.push_back(ch);
                        }
                    }

                    add_line(current, line_no);
                }

                nos::trent parse()
                {
                    index = 0;
                    if (lines.empty())
                        return nos::trent();
                    skip_empty_lines();
                    if (index >= lines.size())
                        return nos::trent();
                    nos::trent res = parse_block(lines[index].indent);
                    skip_empty_lines();
                    if (index < lines.size())
                    {
                        throw nos::yaml::parse_error(
                            lines[index].number,
                            1,
                            "unexpected trailing content");
                    }
                    return res;
                }
            };

            inline void write_indent(std::ostream &os, int indent)
            {
                for (int i = 0; i < indent; ++i)
                    os.put(' ');
            }

            inline void write_node(const nos::trent &tr,
                                   std::ostream &os,
                                   int indent)
            {
                switch (tr.get_type())
                {
                case nos::trent::type::dict: {
                    const auto &dict = tr.unsafe_dict_const();
                    if (dict.empty())
                    {
                        write_indent(os, indent);
                        os << "{}\n";
                        return;
                    }

                    for (const auto &p : dict)
                    {
                        write_indent(os, indent);
                        os << p.first << ":";
                        if (p.second.is_dict() || p.second.is_list())
                        {
                            os << "\n";
                            write_node(p.second, os, indent + 2);
                        }
                        else
                        {
                            os << " " << scalar_to_string(p.second) << "\n";
                        }
                    }
                    break;
                }
                case nos::trent::type::list: {
                    const auto &arr = tr.unsafe_list_const();
                    if (arr.empty())
                    {
                        write_indent(os, indent);
                        os << "[]\n";
                        return;
                    }

                    for (const auto &item : arr)
                    {
                        write_indent(os, indent);
                        os << "-";
                        if (item.is_dict() || item.is_list())
                        {
                            os << "\n";
                            write_node(item, os, indent + 2);
                        }
                        else
                        {
                            os << " " << scalar_to_string(item) << "\n";
                        }
                    }
                    break;
                }
                case nos::trent::type::nil:
                case nos::trent::type::boolean:
                case nos::trent::type::numer:
                case nos::trent::type::string:
                    write_indent(os, indent);
                    os << scalar_to_string(tr) << "\n";
                    break;
                }
            }
        } // namespace detail

        nos::trent parse(const std::string &text)
        {
            detail::parser p(text);
            return p.parse();
        }

        nos::trent parse(const char *text)
        {
            if (text == nullptr)
                return nos::trent();
            return parse(std::string(text));
        }

        nos::trent parse_file(const std::string &path)
        {
            std::ifstream file(path);
            if (!file.is_open())
                throw std::runtime_error("yaml: unable to open file " + path);
            std::stringstream ss;
            ss << file.rdbuf();
            return parse(ss.str());
        }

        void print_to(const nos::trent &tr, std::ostream &os)
        {
            detail::write_node(tr, os, 0);
        }

        std::string to_string(const nos::trent &tr)
        {
            std::ostringstream ss;
            print_to(tr, ss);
            return ss.str();
        }
    }
}
