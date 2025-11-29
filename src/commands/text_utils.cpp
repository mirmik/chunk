#include "commands/text_utils.h"
#include <stdexcept>

namespace text_utils
{

std::string trim(std::string_view view)
{
    if (view.empty())
        return "";

    const char *left = view.data();
    const char *right = view.data() + view.size() - 1;
    const char *end = view.data() + view.size();
    while (left != end &&
           (*left == ' ' || *left == '\n' || *left == '\r' || *left == '\t'))
        ++left;

    if (left == end)
        return "";

    while (left != right &&
           (*right == ' ' || *right == '\n' || *right == '\r' || *right == '\t'))
        --right;

    return std::string(left, (right - left) + 1);
}


PatchLanguage detect_language(const std::string &language)
{
    if (language.empty())
        return PatchLanguage::Unknown;
    std::string lang = language;
    for (char &ch : lang)
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

    if (lang == "c++" || lang == "cpp" || lang == "cxx")
        return PatchLanguage::Cpp;
    if (lang == "python" || lang == "py")
        return PatchLanguage::Python;
    return PatchLanguage::Unknown;
}

std::string normalize_line_for_match(std::string_view view,
                                            PatchLanguage lang)
{
    CodeNormalizer norm(lang);
    return norm.normalize(view);
}

std::vector<MarkerMatch>
find_marker_matches(const std::vector<std::string> &haystack,
                    const std::vector<std::string> &needle,
                    PatchLanguage lang)
{
    std::vector<MarkerMatch> matches;
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
                           PatchLanguage lang,
                           const std::vector<std::string> &before,
                           const std::vector<std::string> &after,
                           const std::vector<MarkerMatch> &candidates)
{
    if (candidates.empty())
        return -1;
    if (before.empty() && after.empty())
        return 0;

    auto match_eq = [&](const std::string &a, const std::string &b) {
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

        if (!before.empty())
        {
            int need = static_cast<int>(before.size());
            if (pos < need)
            {
                ok = false;
            }
            else
            {
                for (int i = 0; i < need; ++i)
                {
                    const std::string &want =
                        before[static_cast<std::size_t>(need - 1 - i)];
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

        if (ok && !after.empty())
        {
            int start = end + 1;
            int need = static_cast<int>(after.size());
            if (start < 0 || start + need > static_cast<int>(lines.size()))
            {
                ok = false;
            }
            else
            {
                for (int i = 0; i < need; ++i)
                {
                    const std::string &want =
                        after[static_cast<std::size_t>(i)];
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
} // namespace text_utils

