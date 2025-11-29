#pragma once

#include "section.h"

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace text_utils
{
struct MarkerMatch
{
    int begin = -1;
    int end = -1;
};


enum class PatchLanguage
{
    Unknown,
    Cpp,
    Python
};


std::string trim(std::string_view view);

PatchLanguage detect_language(const Section *s);

struct CodeNormalizer
{
    PatchLanguage lang;
    bool cpp_in_block_comment = false;
    bool py_in_triple_string = false;
    char py_triple_delim = 0;

    explicit CodeNormalizer(PatchLanguage l)
        : lang(l)
    {
    }

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
            if (i + 2 < len &&
                (data[i] == '\'' || data[i] == '\"') &&
                data[i + 1] == data[i] &&
                data[i + 2] == data[i])
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


std::string normalize_line_for_match(std::string_view view,
                                            PatchLanguage lang);

std::vector<MarkerMatch>
find_marker_matches(const std::vector<std::string> &haystack,
                    const std::vector<std::string> &needle,
                    const Section *section);

int find_best_marker_match(const std::vector<std::string> &lines,
                                  const Section *s,
                                  const std::vector<MarkerMatch> &candidates);
} // namespace text_utils

