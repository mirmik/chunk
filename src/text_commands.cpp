#include "text_commands.h"
#include "text_command_objects.h"

#include <cctype>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace
{
    struct MarkerMatch
    {
        int begin = -1;
        int end = -1;
    };

    std::string trim(std::string_view view)
    {
        if (view.empty())
            return "";

        const char *left = view.data();
        const char *right = view.data() + view.size() - 1;
        const char *end = view.data() + view.size();
        while (left != end && (*left == ' ' || *left == '\n' || *left == '\r' ||
                               *left == '\t'))
            ++left;

        if (left == end)
            return "";

        while (left != right && (*right == ' ' || *right == '\n' ||
                                 *right == '\r' || *right == '\t'))
            --right;

        return std::string(left, (right - left) + 1);
    }

    enum class PatchLanguage
    {
        Unknown,
        Cpp,
        Python
    };

    PatchLanguage detect_language(const Section *s)
    {
        if (!s)
            return PatchLanguage::Unknown;
        std::string lang = s->language;
        if (lang.empty())
            return PatchLanguage::Unknown;

        for (char &ch : lang)
            ch =
                static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        if (lang == "c++" || lang == "cpp" || lang == "cxx")
            return PatchLanguage::Cpp;
        if (lang == "python" || lang == "py")
            return PatchLanguage::Python;
        return PatchLanguage::Unknown;
    }
    struct CodeNormalizer
    {
        PatchLanguage lang;
        bool cpp_in_block_comment = false;
        bool py_in_triple_string = false;
        char py_triple_delim = 0;

        explicit CodeNormalizer(PatchLanguage l) : lang(l) {}

        std::string normalize(std::string_view view)
        {
            return trim(strip_comments(view));
        }

    private:
        std::string strip_comments(std::string_view view)
        {
            if (view.empty())
                return std::string();
            if (lang == PatchLanguage::Cpp)
                return strip_cpp(view);
            if (lang == PatchLanguage::Python)
                return strip_python(view);
            return std::string(view);
        }

        std::string strip_cpp(std::string_view view)
        {
            const char *data = view.data();
            std::size_t len = view.size();
            std::string out;
            out.reserve(len);
            std::size_t i = 0;
            while (i < len)
            {
                if (cpp_in_block_comment)
                {
                    if (i + 1 < len && data[i] == '*' && data[i + 1] == '/')
                    {
                        cpp_in_block_comment = false;
                        i += 2;
                    }
                    else
                    {
                        ++i;
                    }
                    continue;
                }
                if (i + 1 < len && data[i] == '/' && data[i + 1] == '/')
                    break;
                if (i + 1 < len && data[i] == '/' && data[i + 1] == '*')
                {
                    cpp_in_block_comment = true;
                    i += 2;
                    continue;
                }
                out.push_back(data[i]);
                ++i;
            }
            return out;
        }

        std::string strip_python(std::string_view view)
        {
            const char *data = view.data();
            std::size_t len = view.size();
            std::string out;
            out.reserve(len);
            std::size_t i = 0;
            while (i < len)
            {
                if (py_in_triple_string)
                {
                    if (i + 2 < len && data[i] == py_triple_delim &&
                        data[i + 1] == py_triple_delim &&
                        data[i + 2] == py_triple_delim)
                    {
                        py_in_triple_string = false;
                        i += 3;
                    }
                    else
                    {
                        ++i;
                    }
                    continue;
                }
                if (i + 2 < len && (data[i] == '\'' || data[i] == '\"') &&
                    data[i + 1] == data[i] && data[i + 2] == data[i])
                {
                    py_in_triple_string = true;
                    py_triple_delim = data[i];
                    i += 3;
                    continue;
                }
                if (data[i] == '#')
                    break;
                out.push_back(data[i]);
                ++i;
            }
            return out;
        }
    };

    std::string strip_code_comment(std::string_view view, PatchLanguage lang)
    {
        if (view.empty())
            return std::string();

        if (lang == PatchLanguage::Python)
        {
            const char *data = view.data();
            std::size_t len = view.size();
            for (std::size_t i = 0; i < len; ++i)
            {
                if (data[i] == '#')
                    return std::string(data, i);
            }
            return std::string(view);
        }
        if (lang == PatchLanguage::Cpp)
        {
            std::string s(view);
            std::size_t pos = s.find("//");
            if (pos != std::string::npos)
                s.resize(pos);
            return s;
        }
        return std::string(view);
    }

    std::string normalize_line_for_match(std::string_view view,
                                         PatchLanguage lang)
    {
        std::string no_comment = strip_code_comment(view, lang);
        return trim(no_comment);
    }

    std::vector<MarkerMatch>
    find_marker_matches(const std::vector<std::string> &haystack,
                        const std::vector<std::string> &needle,
                        const Section *section)
    {
        std::vector<MarkerMatch> matches;

        PatchLanguage lang = detect_language(section);
        CodeNormalizer pat_norm(lang);
        CodeNormalizer hs_norm(lang);

        std::vector<std::string> pat;
        pat.reserve(needle.size());
        for (const auto &s : needle)
        {
            std::string t = pat_norm.normalize(s);
            if (!t.empty())
                pat.push_back(std::move(t));
        }

        if (pat.empty())
            return matches;
        std::vector<std::string> hs;
        std::vector<int> hs_idx;
        hs.reserve(haystack.size());
        hs_idx.reserve(haystack.size());
        for (int i = 0; i < (int)haystack.size(); ++i)
        {
            std::string t = hs_norm.normalize(haystack[i]);
            if (!t.empty())
            {
                hs.emplace_back(std::move(t));
                hs_idx.push_back(i);
            }
        }

        if (hs.empty() || pat.size() > hs.size())
            return matches;
        const std::size_t n = hs.size();
        const std::size_t m = pat.size();

        for (std::size_t i = 0; i + m <= n; ++i)
        {
            bool ok = true;
            for (std::size_t j = 0; j < m; ++j)
            {
                if (hs[i + j] != pat[j])
                {
                    ok = false;
                    break;
                }
            }
            if (ok)
            {
                int begin = hs_idx[i];
                int end = hs_idx[i + m - 1];
                matches.push_back(MarkerMatch{begin, end});
            }
        }

        return matches;
    }

    int find_best_marker_match(const std::vector<std::string> &lines,
                               const Section *s,
                               const std::vector<MarkerMatch> &candidates)
    {
        if (candidates.empty())
            return -1;
        if (s->before.empty() && s->after.empty())
            return 0;

        PatchLanguage lang = detect_language(s);
        auto match_eq = [&](const std::string &a, const std::string &b)
        {
            return normalize_line_for_match(a, lang) ==
                   normalize_line_for_match(b, lang);
        };

        std::vector<int> strict;

        for (int ci = 0; ci < static_cast<int>(candidates.size()); ++ci)
        {
            const MarkerMatch &mm = candidates[ci];
            int pos = mm.begin;
            int end = mm.end;
            bool ok = true;

            if (!s->before.empty())
            {
                int need = static_cast<int>(s->before.size());
                if (pos < need)
                {
                    ok = false;
                }
                else
                {
                    for (int i = 0; i < need; ++i)
                    {
                        const std::string &want =
                            s->before[static_cast<std::size_t>(need - 1 - i)];
                        const std::string &got =
                            lines[static_cast<std::size_t>(pos - 1 - i)];
                        if (!match_eq(got, want))
                        {
                            ok = false;
                            break;
                        }
                    }
                }
            }

            if (ok && !s->after.empty())
            {
                int start = end + 1;
                int need = static_cast<int>(s->after.size());
                if (start < 0 || start + need > static_cast<int>(lines.size()))
                {
                    ok = false;
                }
                else
                {
                    for (int i = 0; i < need; ++i)
                    {
                        const std::string &want =
                            s->after[static_cast<std::size_t>(i)];
                        const std::string &got =
                            lines[static_cast<std::size_t>(start + i)];
                        if (!match_eq(got, want))
                        {
                            ok = false;
                            break;
                        }
                    }
                }
            }

            if (ok)
                strict.push_back(ci);
        }

        if (strict.empty())
            throw std::runtime_error("strict marker context not found");

        if (strict.size() > 1)
            throw std::runtime_error("strict marker match is ambiguous");

        return strict.front();
    }

} // namespace

namespace text_commands_detail
{
    SimpleInsertCommand::SimpleInsertCommand(const Section &s, bool prepend)
        : section(s), prepend_mode(prepend)
    {
    }

    void SimpleInsertCommand::execute(std::vector<std::string> &lines)
    {
        if (prepend_mode)
            lines.insert(
                lines.begin(), section.payload.begin(), section.payload.end());
        else
            lines.insert(
                lines.end(), section.payload.begin(), section.payload.end());
    }

    MarkerTextCommand::MarkerTextCommand(const Section &s, std::string path)
        : section(s), filepath(std::move(path))
    {
    }

    void MarkerTextCommand::execute(std::vector<std::string> &lines)
    {
        if (section.marker.empty())
            throw std::runtime_error("empty marker in text command for file: " +
                                     filepath);

        std::vector<MarkerMatch> matches =
            find_marker_matches(lines, section.marker, &section);
        if (matches.empty())
            throw std::runtime_error(
                "text marker not found for file: " + filepath +
                "\ncommand: " + section.command + "\n");

        int mindex = find_best_marker_match(lines, &section, matches);
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

    std::vector<std::string>
    MarkerTextCommand::prepare_payload(const std::vector<std::string> &lines,
                                       std::size_t begin) const
    {
        if (!should_indent_payload() || !section.indent_from_marker)
            return section.payload;

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
        adjusted.reserve(section.payload.size());
        for (const auto &ln : section.payload)
        {
            if (ln.empty())
                adjusted.push_back(ln);
            else
                adjusted.push_back(prefix + ln);
        }
        return adjusted;
    }

    InsertAfterTextCommand::InsertAfterTextCommand(const Section &s,
                                                   std::string path)
        : MarkerTextCommand(s, std::move(path))
    {
    }

    void InsertAfterTextCommand::apply(std::vector<std::string> &lines,
                                       std::size_t begin,
                                       std::size_t end,
                                       const std::vector<std::string> &payload)
    {
        (void)begin;
        std::size_t pos = end + 1;
        lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(pos),
                     payload.begin(),
                     payload.end());
    }

    InsertBeforeTextCommand::InsertBeforeTextCommand(const Section &s,
                                                     std::string path)
        : MarkerTextCommand(s, std::move(path))
    {
    }

    void InsertBeforeTextCommand::apply(std::vector<std::string> &lines,
                                        std::size_t begin,
                                        std::size_t /*end*/,
                                        const std::vector<std::string> &payload)
    {
        lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(begin),
                     payload.begin(),
                     payload.end());
    }

    ReplaceTextCommand::ReplaceTextCommand(const Section &s, std::string path)
        : MarkerTextCommand(s, std::move(path))
    {
    }

    void ReplaceTextCommand::apply(std::vector<std::string> &lines,
                                   std::size_t begin,
                                   std::size_t end,
                                   const std::vector<std::string> &payload)
    {
        auto it_begin = lines.begin() + static_cast<std::ptrdiff_t>(begin);
        auto it_end = lines.begin() + static_cast<std::ptrdiff_t>(end + 1);
        lines.erase(it_begin, it_end);
        lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(begin),
                     payload.begin(),
                     payload.end());
    }

    DeleteTextCommand::DeleteTextCommand(const Section &s, std::string path)
        : MarkerTextCommand(s, std::move(path))
    {
    }

    void DeleteTextCommand::apply(std::vector<std::string> &lines,
                                  std::size_t begin,
                                  std::size_t end,
                                  const std::vector<std::string> &payload)
    {
        (void)payload;
        auto it_begin = lines.begin() + static_cast<std::ptrdiff_t>(begin);
        auto it_end = lines.begin() + static_cast<std::ptrdiff_t>(end + 1);
        lines.erase(it_begin, it_end);
    }
} // namespace text_commands_detail

namespace
{
    const std::unordered_map<std::string,
                             text_commands_detail::CommandFactory> &
    text_command_registry()
    {
        using namespace text_commands_detail;

        static const std::unordered_map<std::string, CommandFactory> registry = {
            {"prepend-text",
             [](const Section &s, const std::string &)
             { return std::make_unique<SimpleInsertCommand>(s, true); }},
            {"append-text",
             [](const Section &s, const std::string &)
             { return std::make_unique<SimpleInsertCommand>(s, false); }},
            {"insert-after-text",
             [](const Section &s, const std::string &filepath) {
                 return std::make_unique<InsertAfterTextCommand>(s, filepath);
             }},
            {"insert-before-text",
             [](const Section &s, const std::string &filepath) {
                 return std::make_unique<InsertBeforeTextCommand>(s, filepath);
             }},
            {"replace-text",
             [](const Section &s, const std::string &filepath)
             { return std::make_unique<ReplaceTextCommand>(s, filepath); }},
            {"delete-text",
             [](const Section &s, const std::string &filepath)
             { return std::make_unique<DeleteTextCommand>(s, filepath); }},
        };

        return registry;
    }
} // namespace

std::unique_ptr<Command>
create_text_command(const Section &section, const std::string &filepath)
{
    const auto &registry = text_command_registry();
    auto it = registry.find(section.command);
    if (it == registry.end())
        throw std::runtime_error("unknown text command: " + section.command);
    return it->second(section, filepath);
}

void apply_text_commands(const std::string &filepath,
                         std::vector<std::string> &lines,
                         const std::vector<const Section *> &sections)
{
    for (const Section *s : sections)
    {
        auto cmd = create_text_command(*s, filepath);
        cmd->execute(lines);
    }
}
