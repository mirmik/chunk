#include "symbols.h"

#include <cctype>

PythonSymbolFinder::PythonSymbolFinder(const std::string &text)
{
    m_lines.clear();
    std::string current;
    current.reserve(80);

    for (char c : text)
    {
        if (c == '\n')
        {
            m_lines.push_back(current);
            current.clear();
        }
        else if (c != '\r')
        {
            current.push_back(c);
        }
    }
    m_lines.push_back(current);
}

int PythonSymbolFinder::calc_indent(const std::string &line)
{
    int indent = 0;
    for (char c : line)
    {
        if (c == ' ')
            ++indent;
        else if (c == '\t')
            indent += 4; // грубая оценка, но устойчиво для сравнения
        else
            break;
    }
    return indent;
}

std::size_t PythonSymbolFinder::first_code_pos(const std::string &line)
{
    std::size_t i = 0;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
        ++i;
    return i;
}

bool PythonSymbolFinder::find_class_internal(const std::string &class_name,
                                             Region &out,
                                             int &class_indent) const
{
    const int n = static_cast<int>(m_lines.size());
    for (int i = 0; i < n; ++i)
    {
        const std::string &line = m_lines[static_cast<std::size_t>(i)];
        std::size_t pos = first_code_pos(line);
        if (pos >= line.size())
            continue;

        // комментарии / shebang
        if (line[pos] == '#')
            continue;

        if (line.compare(pos, 5, "class") != 0)
            continue;

        char after = (pos + 5 < line.size()) ? line[pos + 5] : '\0';
        if (!(after == '\0' ||
              std::isspace(static_cast<unsigned char>(after)) || after == '(' ||
              after == ':'))
            continue;

        std::size_t p = pos + 5;
        while (p < line.size() &&
               std::isspace(static_cast<unsigned char>(line[p])))
            ++p;

        std::size_t name_start = p;
        while (p < line.size() &&
               (std::isalnum(static_cast<unsigned char>(line[p])) ||
                line[p] == '_'))
            ++p;

        if (name_start == p)
            continue;

        std::string name = line.substr(name_start, p - name_start);
        if (name != class_name)
            continue;

        class_indent = calc_indent(line);
        int last_body = i;

        for (int k = i + 1; k < n; ++k)
        {
            const std::string &l2 = m_lines[static_cast<std::size_t>(k)];
            std::size_t pos2 = first_code_pos(l2);
            if (pos2 >= l2.size())
                continue; // пустая строка в теле

            int ind2 = calc_indent(l2);
            if (ind2 <= class_indent)
                break; // dedent — выходим из класса

            last_body = k;
        }

        out.start_line = i;
        out.end_line = last_body;
        return true;
    }

    return false;
}

bool PythonSymbolFinder::find_class(const std::string &class_name,
                                    Region &out) const
{
    int indent = 0;
    return find_class_internal(class_name, out, indent);
}

bool PythonSymbolFinder::find_method(const std::string &class_name,
                                     const std::string &method_name,
                                     Region &out) const
{
    Region class_region;
    int class_indent = 0;
    if (!find_class_internal(class_name, class_region, class_indent))
        return false;

    const int start = class_region.start_line;
    const int end = class_region.end_line;

    // Определяем базовый уровень отступа для членов класса
    int member_indent = -1;
    for (int i = start + 1; i <= end; ++i)
    {
        const std::string &line = m_lines[static_cast<std::size_t>(i)];
        std::size_t pos = first_code_pos(line);
        if (pos >= line.size())
            continue;
        if (line[pos] == '#')
            continue;

        int ind = calc_indent(line);
        if (ind <= class_indent)
            continue;

        member_indent = ind;
        break;
    }

    if (member_indent < 0)
        return false; // пустой класс

    const int n = static_cast<int>(m_lines.size());

    for (int i = start + 1; i <= end && i < n; ++i)
    {
        const std::string &line = m_lines[static_cast<std::size_t>(i)];
        std::size_t pos = first_code_pos(line);
        if (pos >= line.size())
            continue;

        int ind = calc_indent(line);
        if (ind != member_indent)
            continue;

        if (line[pos] == '#')
            continue;

        // def / async def
        std::size_t p = pos;
        bool is_async = false;

        if (line.compare(p, 5, "async") == 0 &&
            (p + 5 >= line.size() ||
             std::isspace(static_cast<unsigned char>(line[p + 5]))))
        {
            is_async = true;
            p += 5;
            while (p < line.size() &&
                   std::isspace(static_cast<unsigned char>(line[p])))
                ++p;
        }

        if (line.compare(p, 3, "def") != 0 ||
            (p + 3 < line.size() &&
             !std::isspace(static_cast<unsigned char>(line[p + 3]))))
        {
            continue;
        }

        p += 3;
        while (p < line.size() &&
               std::isspace(static_cast<unsigned char>(line[p])))
            ++p;

        std::size_t name_start = p;
        while (p < line.size() &&
               (std::isalnum(static_cast<unsigned char>(line[p])) ||
                line[p] == '_'))
            ++p;

        if (name_start == p)
            continue;

        std::string name = line.substr(name_start, p - name_start);
        if (name != method_name)
            continue;

        // Нашли нужный метод
        int decl_start = i;

        // Захватываем декораторы над методом (тем же отступом)
        for (int j = i - 1; j > start; --j)
        {
            const std::string &pline = m_lines[static_cast<std::size_t>(j)];
            std::size_t ppos = first_code_pos(pline);
            if (ppos >= pline.size())
                break;

            int pind = calc_indent(pline);
            if (pind != member_indent)
                break;
            if (pline[ppos] != '@')
                break;

            decl_start = j;
        }

        int last_body = i;
        for (int k = i + 1; k <= end && k < n; ++k)
        {
            const std::string &l2 = m_lines[static_cast<std::size_t>(k)];
            std::size_t pos2 = first_code_pos(l2);
            if (pos2 >= l2.size())
                continue;

            int ind2 = calc_indent(l2);

            if (ind2 <= class_indent)
                break; // вышли из класса
            if (ind2 < member_indent)
                break; // вышли из метода

            // Новый метод / класс на том же уровне — заканчиваем текущий
            if (ind2 == member_indent)
            {
                bool is_new_block = false;

                if (l2.compare(pos2, 5, "class") == 0 &&
                    (pos2 + 5 >= l2.size() ||
                     std::isspace(static_cast<unsigned char>(l2[pos2 + 5]))))
                {
                    is_new_block = true;
                }
                else if (l2.compare(pos2, 3, "def") == 0 &&
                         (pos2 + 3 >= l2.size() ||
                          std::isspace(
                              static_cast<unsigned char>(l2[pos2 + 3]))))
                {
                    is_new_block = true;
                }
                else if (l2.compare(pos2, 5, "async") == 0)
                {
                    std::size_t q = pos2 + 5;
                    while (q < l2.size() &&
                           std::isspace(static_cast<unsigned char>(l2[q])))
                        ++q;
                    if (l2.compare(q, 3, "def") == 0 &&
                        (q + 3 >= l2.size() ||
                         std::isspace(static_cast<unsigned char>(l2[q + 3]))))
                    {
                        is_new_block = true;
                    }
                }

                if (is_new_block)
                    break;
            }

            last_body = k;
        }

        out.start_line = decl_start;
        out.end_line = last_body;
        return true;
    }

    return false;
}
