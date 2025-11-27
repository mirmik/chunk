#include <cctype>
#include <cstdlib>
#include <fstream>
#include <yaml/yaml.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace nos
{
    namespace yaml
    {
        namespace detail
        {
            struct line
            {
                int indent = 0;
                std::string text;
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

            class parser
            {
                std::vector<line> lines;
                size_t index = 0;

                void add_line(std::string line_text)
                {
                    if (line_text.empty())
                        return;

                    size_t pos = 0;
                    int indent = 0;
                    while (pos < line_text.size() &&
                           (line_text[pos] == ' ' || line_text[pos] == '\t'))
                    {
                        indent += (line_text[pos] == '\t') ? 4 : 1;
                        ++pos;
                    }

                    std::string content = line_text.substr(pos);

                    bool in_single = false;
                    bool in_double = false;
                    std::string filtered;
                    filtered.reserve(content.size());

                    for (size_t i = 0; i < content.size(); ++i)
                    {
                        char ch = content[i];

                        if (ch == '"' && !in_single)
                        {
                            in_double = !in_double;
                            filtered.push_back(ch);
                            continue;
                        }

                        if (ch == '\'' && !in_double)
                        {
                            if (in_single && i + 1 < content.size() &&
                                content[i + 1] == '\'')
                            {
                                filtered.push_back('\'');
                                ++i;
                                continue;
                            }
                            in_single = !in_single;
                            filtered.push_back(ch);
                            continue;
                        }

                        if (ch == '#' && !in_single && !in_double)
                            break;

                        filtered.push_back(ch);
                    }

                    content = trim(filtered);
                    if (content.empty())
                        return;

                    lines.push_back({indent, content});
                }

                static std::string parse_single_quoted(const std::string &text)
                {
                    std::string result;
                    for (size_t i = 1; i < text.size(); ++i)
                    {
                        char ch = text[i];
                        if (ch == '\'')
                        {
                            if (i + 1 < text.size() && text[i + 1] == '\'')
                            {
                                result.push_back('\'');
                                ++i;
                                continue;
                            }
                            break;
                        }
                        result.push_back(ch);
                    }
                    return result;
                }

                static std::string parse_double_quoted(const std::string &text)
                {
                    std::string result;
                    for (size_t i = 1; i < text.size(); ++i)
                    {
                        char ch = text[i];
                        if (ch == '"')
                            break;
                        if (ch == '\\')
                        {
                            if (i + 1 >= text.size())
                                break;
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

                static std::string to_lower(const std::string &s)
                {
                    std::string out = s;
                    for (auto &ch : out)
                        ch = static_cast<char>(
                            std::tolower(static_cast<unsigned char>(ch)));
                    return out;
                }

                static nos::trent parse_scalar_text(const std::string &text)
                {
                    std::string trimmed = trim(text);
                    if (trimmed.empty())
                        return nos::trent::nil();

                    if (trimmed.front() == '"' && trimmed.back() == '"' &&
                        trimmed.size() >= 2)
                    {
                        return parse_double_quoted(trimmed);
                    }

                    if (trimmed.front() == '\'' && trimmed.back() == '\'' &&
                        trimmed.size() >= 2)
                    {
                        return parse_single_quoted(trimmed);
                    }

                    auto lower = to_lower(trimmed);
                    if (lower == "true")
                        return nos::trent(true);
                    if (lower == "false")
                        return nos::trent(false);
                    if (lower == "null" || lower == "~")
                        return nos::trent::nil();

                    char *endptr = nullptr;
                    double value = std::strtod(trimmed.c_str(), &endptr);
                    if (endptr && *endptr == '\0' && endptr != trimmed.c_str())
                    {
                        return nos::trent(value);
                    }

                    return nos::trent(trimmed);
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
                            return i;
                    }
                    return std::string::npos;
                }

                bool is_sequence_line(size_t idx, int indent) const
                {
                    if (idx >= lines.size())
                        return false;
                    const auto &ln = lines[idx];
                    return ln.indent == indent && !ln.text.empty() &&
                           ln.text[0] == '-';
                }

                bool is_mapping_line(size_t idx, int indent) const
                {
                    if (idx >= lines.size())
                        return false;
                    const auto &ln = lines[idx];
                    if (ln.indent != indent)
                        return false;
                    return find_unescaped_colon(ln.text) != std::string::npos;
                }

                nos::trent parse_block(int indent)
                {
                    if (index >= lines.size())
                        return nos::trent();

                    if (lines[index].indent < indent)
                        return nos::trent();

                    if (is_sequence_line(index, lines[index].indent))
                        return parse_sequence(lines[index].indent);

                    if (is_mapping_line(index, lines[index].indent))
                        return parse_mapping(lines[index].indent);

                    auto scalar = parse_scalar_text(lines[index].text);
                    ++index;
                    return scalar;
                }

                nos::trent parse_sequence(int indent)
                {
                    nos::trent arr;
                    arr.as_list();

                    while (index < lines.size())
                    {
                        const auto &ln = lines[index];
                        if (ln.indent != indent || ln.text.empty() ||
                            ln.text[0] != '-')
                            break;

                        std::string rest = trim(ln.text.substr(1));
                        ++index;

                        nos::trent element;
                        bool element_initialized = false;

                        if (!rest.empty())
                        {
                            size_t colon = find_unescaped_colon(rest);
                            if (colon != std::string::npos)
                            {
                                element.init(nos::trent::type::dict);
                                std::string key = trim(rest.substr(0, colon));
                                std::string value_text =
                                    trim(rest.substr(colon + 1));
                                if (!key.empty())
                                {
                                    if (value_text.empty())
                                        element[key] = nos::trent::nil();
                                    else
                                        element[key] =
                                            parse_scalar_text(value_text);
                                }
                                element_initialized = true;
                            }
                            else
                            {
                                element = parse_scalar_text(rest);
                                element_initialized = true;
                            }
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
                        if (ln.indent != indent)
                            break;

                        size_t colon = find_unescaped_colon(ln.text);
                        if (colon == std::string::npos)
                            break;

                        std::string key = trim(ln.text.substr(0, colon));
                        std::string value_text =
                            trim(ln.text.substr(colon + 1));

                        ++index;

                        if (!value_text.empty())
                        {
                            obj[key] = parse_scalar_text(value_text);
                            continue;
                        }

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

                    for (char ch : text)
                    {
                        if (ch == '\r')
                            continue;
                        if (ch == '\n')
                        {
                            add_line(current);
                            current.clear();
                        }
                        else
                        {
                            current.push_back(ch);
                        }
                    }

                    add_line(current);
                }

                nos::trent parse()
                {
                    index = 0;
                    if (lines.empty())
                        return nos::trent();
                    return parse_block(lines.front().indent);
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
