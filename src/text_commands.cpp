#include "text_commands.h"
#include "command_parse_helpers.h"
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
    SimpleInsertCommand::SimpleInsertCommand(const std::string &name,
                                             bool prepend)
        : Command(name), prepend_mode(prepend)
    {
    }

    void SimpleInsertCommand::parse(const nos::trent &tr)
    {
        section_.filepath = command_parse::get_scalar(tr, "path");
        if (section_.filepath.empty())
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'path' key");

        section_.payload = command_parse::split_scalar_lines(
            command_parse::get_scalar(tr, "payload"));
        if (section_.payload.empty())
            throw std::runtime_error("YAML patch: text op '" + command_name() +
                                     "' for file '" + section_.filepath +
                                     "' requires 'payload'");

        section_.comment = command_parse::get_scalar(tr, "comment");
    }

    void SimpleInsertCommand::execute(std::vector<std::string> &lines)
    {
        if (prepend_mode)
            lines.insert(lines.begin(),
                         section_.payload.begin(),
                         section_.payload.end());
        else
            lines.insert(
                lines.end(), section_.payload.begin(), section_.payload.end());
    }

    MarkerTextCommand::MarkerTextCommand(const std::string &name)
        : Command(name)
    {
    }

    void MarkerTextCommand::parse(const nos::trent &tr)
    {
        section_.filepath = command_parse::get_scalar(tr, "path");
        if (section_.filepath.empty())
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'path' key");

        section_.indent_from_marker = command_parse::parse_indent_from_options(
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
            throw std::runtime_error("YAML patch: text op '" + command_name() +
                                     "' for file '" + section_.filepath +
                                     "' requires 'marker'");

        section_.marker = command_parse::split_scalar_lines(marker_text);

        if (!before_text.empty())
            section_.before = command_parse::split_scalar_lines(before_text);
        if (!after_text.empty())
            section_.after = command_parse::split_scalar_lines(after_text);
        if (command_name() != "delete-text" && !payload_text.empty())
            section_.payload = command_parse::split_scalar_lines(payload_text);
    }

    void MarkerTextCommand::execute(std::vector<std::string> &lines)
    {
        if (section_.marker.empty())
            throw std::runtime_error("empty marker in text command for file: " +
                                     section_.filepath);

        std::vector<MarkerMatch> matches =
            find_marker_matches(lines, section_.marker, &section_);
        if (matches.empty())
            throw std::runtime_error(
                "text marker not found for file: " + section_.filepath +
                "\ncommand: " + section_.command + "\n");

        int mindex = find_best_marker_match(lines, &section_, matches);
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

    InsertAfterTextCommand::InsertAfterTextCommand()
        : MarkerTextCommand("insert-after-text")
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

    InsertBeforeTextCommand::InsertBeforeTextCommand()
        : MarkerTextCommand("insert-before-text")
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

    ReplaceTextCommand::ReplaceTextCommand() : MarkerTextCommand("replace-text")
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

    DeleteTextCommand::DeleteTextCommand() : MarkerTextCommand("delete-text") {}

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

        static const std::unordered_map<std::string, CommandFactory> registry =
            {
                {"prepend-text",
                 []() {
                     return std::make_unique<SimpleInsertCommand>(
                         "prepend-text", true);
                 }},
                {"append-text",
                 []() {
                     return std::make_unique<SimpleInsertCommand>("append-text",
                                                                  false);
                 }},
                {"insert-after-text",
                 []() { return std::make_unique<InsertAfterTextCommand>(); }},
                {"insert-before-text",
                 []() { return std::make_unique<InsertBeforeTextCommand>(); }},
                {"replace-text",
                 []() { return std::make_unique<ReplaceTextCommand>(); }},
                {"delete-text",
                 []() { return std::make_unique<DeleteTextCommand>(); }},
            };

        return registry;
    }
} // namespace

std::unique_ptr<Command> create_text_command(const Section &section,
                                             const std::string &filepath)
{
    const auto &registry = text_command_registry();
    auto it = registry.find(section.command);
    if (it == registry.end())
        throw std::runtime_error("unknown text command: " + section.command);
    auto cmd = it->second();
    cmd->load_section(section);
    (void)filepath;
    return cmd;
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
