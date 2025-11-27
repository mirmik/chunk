#include "apply_chunk_v2.h"
#include "symbols.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <yaml/yaml.h>

namespace fs = std::filesystem;

struct Section
{
    std::string filepath;
    std::string command;
    int a = -1;
    int b = -1;
    std::vector<std::string> payload;
    std::vector<std::string> marker;
    std::vector<std::string> before; // контекст до маркера (BEFORE:)
    std::vector<std::string> after;  // контекст после маркера (AFTER:)
    int seq = 0;
    std::string arg1; // доп. аргументы команды (например, имя класса)
    std::string arg2; // второй аргумент (например, имя метода)

    bool indent_from_marker = true;
};


static std::vector<std::string> read_file_lines(const fs::path &p)
{
    std::ifstream in(p);
    if (!in)
        throw std::runtime_error("cannot open file: " + p.string());

    std::vector<std::string> out;
    std::string line;
    while (std::getline(in, line))
        out.push_back(line);

    return out;
}

static void write_file_lines(const fs::path &p,
                             const std::vector<std::string> &lines)
{
    std::ofstream out(p, std::ios::trunc);
    if (!out)
        throw std::runtime_error("cannot write file: " + p.string());

    for (const auto &s : lines)
        out << s << "\n";
}

std::string trim(const std::string_view &view)
{
    if (view.size() == 0)
        return "";

    const char *left = view.data();
    const char *right = view.data() + view.size() - 1;
    const char *end = view.data() + view.size();

    while (left != end &&
           (*left == ' ' || *left == '\n' || *left == '\r' || *left == '\t'))
        ++left;

    if (left == end)
        return "";

    while (left != right && (*right == ' ' || *right == '\n' ||
                             *right == '\r' || *right == '\t'))
        --right;

    return std::string(left, (right - left) + 1);
}

static bool is_text_command(const std::string &cmd)
{
    return cmd == "insert-after-text" || cmd == "insert-before-text" ||
           cmd == "replace-text" || cmd == "delete-text";
}

static bool is_symbol_command(const std::string &cmd)
{
    return cmd == "replace-cpp-method" || cmd == "replace-cpp-class" ||
           cmd == "replace-py-method" || cmd == "replace-py-class";
}

static std::vector<std::string> split_scalar_lines(const std::string &text)
{
    std::vector<std::string> result;
    if (text.empty())
        return result;

    std::size_t start = 0;
    const std::size_t n = text.size();

    while (true)
    {
        std::size_t pos = text.find('\n', start);
        if (pos == std::string::npos)
        {
            // Последний фрагмент без завершающего '\n'
            if (start < n)
                result.emplace_back(text.substr(start));
            break;
        }

        result.emplace_back(text.substr(start, pos - start));
        start = pos + 1;

        if (start >= n)
        {
            // Строка заканчивалась на '\n' — не создаём лишнюю пустую строку.
            break;
        }
    }

    return result;
}

static std::vector<Section> parse_yaml_patch_text(const std::string &text)
{
    using nos::trent;

    trent root = nos::yaml::parse(text);

    const trent *ops_node = nullptr;

    if (root.is_dict())
    {
        auto &dict = root.as_dict();
        auto it = dict.find("operations");
        if (it == dict.end())
            throw std::runtime_error("YAML patch: missing 'operations' key");
        ops_node = &it->second;
    }
    else if (root.is_list())
    {
        // Допускаем формат: в корне сразу список операций
        ops_node = &root;
    }
    else
    {
        throw std::runtime_error(
            "YAML patch: root must be mapping or sequence");
    }

    if (!ops_node->is_list())
        throw std::runtime_error("YAML patch: 'operations' must be a sequence");

    const auto &ops = ops_node->as_list();
    std::vector<Section> sections;
    sections.reserve(ops.size());

    int seq = 0;

    for (const trent &op_node : ops)
    {
        if (!op_node.is_dict())
            throw std::runtime_error("YAML patch: each operation must be a mapping");

        const auto &m = op_node.as_dict();

        auto it_path = m.find("path");
        auto it_op   = m.find("op");
        if (it_path == m.end())
            throw std::runtime_error("YAML patch: operation missing 'path'");
        if (it_op == m.end())
            throw std::runtime_error("YAML patch: operation missing 'op'");

        Section s;
        s.filepath = it_path->second.as_string();
        if (s.filepath.empty())
            throw std::runtime_error("YAML patch: 'path' must not be empty");

        std::string op_name = it_op->second.as_string();

        if (op_name == "create_file")
            s.command = "create-file";
        else if (op_name == "delete_file")
            s.command = "delete-file";
        else if (op_name == "insert_after_text")
            s.command = "insert-after-text";
        else if (op_name == "insert_before_text")
            s.command = "insert-before-text";
        else if (op_name == "replace_text")
            s.command = "replace-text";
        else if (op_name == "delete_text")
            s.command = "delete-text";
        else
            throw std::runtime_error("YAML patch: unknown op: " + op_name);

        s.seq = seq++;

        auto get_scalar = [&](const char *key) -> std::string
        {
            auto it = m.find(key);
            if (it == m.end())
                return std::string();
            const trent &v = it->second;
            if (v.is_nil())
                return std::string();
            return v.as_string();
        };

        std::string marker_text  = get_scalar("marker");
        std::string before_text  = get_scalar("before");
        std::string after_text   = get_scalar("after");
        std::string payload_text = get_scalar("payload");

        // Опции (используем только indent)
        auto it_opts = m.find("options");
        if (it_opts != m.end() && !it_opts->second.is_nil())
        {
            const auto &opts = it_opts->second.as_dict();
            auto it_ind = opts.find("indent");
            if (it_ind != opts.end())
            {
                std::string mode = it_ind->second.as_string();

                // явное выключение авто-идентации
                if (mode == "none" || mode == "as-is")
                {
                    s.indent_from_marker = false;
                }
                // явно включить (на случай если кто-то всё ещё пишет from-marker)
                else if (mode == "from-marker" || mode == "marker" || mode == "auto")
                {
                    s.indent_from_marker = true;
                }
                else
                {
                    throw std::runtime_error(
                        "YAML patch: unknown indent mode: " + mode);
                }
            }
        }

        if (s.command == "create-file")
        {
            if (!payload_text.empty())
                s.payload = split_scalar_lines(payload_text);
        }
        else if (s.command == "delete-file")
        {
            // Ничего больше не нужно.
        }
        else if (is_text_command(s.command))
        {
            if (marker_text.empty())
                throw std::runtime_error(
                    "YAML patch: text op '" + op_name +
                    "' for file '" + s.filepath + "' requires 'marker'");

            s.marker = split_scalar_lines(marker_text);

            if (!before_text.empty())
                s.before = split_scalar_lines(before_text);
            if (!after_text.empty())
                s.after = split_scalar_lines(after_text);

            if (s.command != "delete-text")
            {
                if (!payload_text.empty())
                    s.payload = split_scalar_lines(payload_text);
            }
        }
        else
        {
            // сюда попадать не должны (create/delete уже обработаны)
        }

        sections.emplace_back(std::move(s));
    }

    return sections;
}


static int find_subsequence(const std::vector<std::string> &haystack,
                            const std::vector<std::string> &needle)
{
    if (needle.empty() || needle.size() > haystack.size())
        return -1;

    const std::size_t n = haystack.size();
    const std::size_t m = needle.size();

    for (std::size_t i = 0; i + m <= n; ++i)
    {
        bool ok = true;
        for (std::size_t j = 0; j < m; ++j)
        {
            std::string h = trim(haystack[i + j]);
            std::string nn = trim(needle[j]);
            if (h != nn)
            {
                ok = false;
                break;
            }
        }
        if (ok)
            return static_cast<int>(i);
    }

    return -1;
}

// Строгий выбор позиции маркера с учётом BEFORE/AFTER.
// Никакого fuzzy, только точное позиционное совпадение.
static int find_best_marker_match(const std::vector<std::string> &lines,
                                  const Section *s,
                                  const std::vector<int> &candidates)
{
    if (candidates.empty())
        return -1;

    // Нет дополнительного контекста — ведём себя как раньше.
    if (s->before.empty() && s->after.empty())
        return candidates.front();

    auto trim_eq = [&](const std::string &a, const std::string &b)
    { return trim(a) == trim(b); };

    std::vector<int> strict;

    for (int pos : candidates)
    {
        bool ok = true;

        // BEFORE: строки сразу над маркером
        if (!s->before.empty())
        {
            int need = static_cast<int>(s->before.size());
            if (pos < need)
            {
                ok = false;
            }
            else
            {
                // Последняя строка BEFORE должна быть непосредственно над
                // первой строкой маркера.
                for (int i = 0; i < need; ++i)
                {
                    const std::string &want =
                        s->before[static_cast<std::size_t>(need - 1 - i)];
                    const std::string &got =
                        lines[static_cast<std::size_t>(pos - 1 - i)];
                    if (!trim_eq(got, want))
                    {
                        ok = false;
                        break;
                    }
                }
            }
        }

        // AFTER: строки сразу под маркером
        if (ok && !s->after.empty())
        {
            int start = pos + static_cast<int>(s->marker.size());
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
                    if (!trim_eq(got, want))
                    {
                        ok = false;
                        break;
                    }
                }
            }
        }

        if (ok)
            strict.push_back(pos);
    }

    if (strict.empty())
        throw std::runtime_error("strict marker context not found");

    if (strict.size() > 1)
        throw std::runtime_error("strict marker match is ambiguous");

    return strict.front();
}

static void apply_text_commands(const std::string &filepath,
                                std::vector<std::string> &lines,
                                const std::vector<const Section *> &sections)
{
    for (const Section *s : sections)
    {
        if (s->marker.empty())
            throw std::runtime_error("empty marker in text command for file: " +
                                     filepath);

        // Собираем все вхождения маркера
        std::vector<int> candidates;
        {
            int base = 0;
            while (base < static_cast<int>(lines.size()))
            {
                std::vector<std::string> sub(lines.begin() + base, lines.end());
                int idx = find_subsequence(sub, s->marker);
                if (idx < 0)
                    break;
                candidates.push_back(base + idx);
                base += idx + 1;
            }
        }

        if (candidates.empty())
            throw std::runtime_error(
                "text marker not found for file: " + filepath +
                "\ncommand: " + s->command + "\n");

        int idx = find_best_marker_match(lines, s, candidates);
        if (idx < 0)
            throw std::runtime_error("cannot locate marker uniquely");

        std::size_t pos = static_cast<std::size_t>(idx);

        // Локальная копия payload, чтобы можно было добавить отступ
        std::vector<std::string> payload = s->payload;

        if (s->indent_from_marker &&
            (s->command == "insert-after-text" ||
             s->command == "insert-before-text" ||
             s->command == "replace-text"))
        {
            // Берём отступ первой строки маркера
            std::string prefix;
            if (pos < lines.size())
            {
                const std::string &marker_line = lines[pos];
                std::size_t j = 0;
                while (j < marker_line.size() &&
                       (marker_line[j] == ' ' || marker_line[j] == '\t'))
                    ++j;
                prefix.assign(marker_line, 0, j);
            }

            std::vector<std::string> adjusted;
            adjusted.reserve(payload.size());
            for (const auto &ln : payload)
            {
                if (ln.empty())
                    adjusted.push_back(ln);
                else
                    adjusted.push_back(prefix + ln);
            }
            payload.swap(adjusted);
        }

        if (s->command == "insert-after-text")
        {
            pos += s->marker.size();
            lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(pos),
                         payload.begin(),
                         payload.end());
        }
        else if (s->command == "insert-before-text")
        {
            lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(pos),
                         payload.begin(),
                         payload.end());
        }
        else if (s->command == "replace-text")
        {
            auto begin = lines.begin() + static_cast<std::ptrdiff_t>(pos);
            auto end = begin + static_cast<std::ptrdiff_t>(s->marker.size());
            lines.erase(begin, end);
            lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(pos),
                         payload.begin(),
                         payload.end());
        }
        else if (s->command == "delete-text")
        {
            auto begin = lines.begin() + static_cast<std::ptrdiff_t>(pos);
            auto end = begin + static_cast<std::ptrdiff_t>(s->marker.size());
            lines.erase(begin, end);
        }
        else
        {
            throw std::runtime_error("unknown text command: " + s->command);
        }
    }
}

static std::string join_lines(const std::vector<std::string> &lines)
{
    if (lines.empty())
        return std::string();

    std::string text;
    std::size_t total = 0;
    for (const auto &s : lines)
        total += s.size() + 1;
    text.reserve(total);

    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        text += lines[i];
        if (i + 1 < lines.size())
            text += '\n';
    }

    return text;
}

static void apply_symbol_commands(const std::string &filepath,
                                  std::vector<std::string> &lines,
                                  const std::vector<const Section *> &sections)
{
    for (const Section *s : sections)
    {
        // Всегда работаем с текущей версией файла
        std::string text = join_lines(lines);

        if (s->command == "replace-cpp-class" ||
            s->command == "replace-cpp-method")
        {
            CppSymbolFinder finder(text);

            if (s->command == "replace-cpp-class")
            {
                if (s->arg1.empty())
                    throw std::runtime_error(
                        "replace-cpp-class: missing class name for file: " +
                        filepath);

                Region r;
                if (!finder.find_class(s->arg1, r))
                    throw std::runtime_error(
                        "replace-cpp-class: class not found: " + s->arg1 +
                        " in file: " + filepath);

                if (r.start_line < 0 || r.end_line < r.start_line ||
                    r.end_line >= static_cast<int>(lines.size()))
                    throw std::runtime_error(
                        "replace-cpp-class: invalid region");

                auto begin = lines.begin() + r.start_line;
                auto end = lines.begin() + (r.end_line + 1);
                lines.erase(begin, end);
                lines.insert(lines.begin() + r.start_line,
                             s->payload.begin(),
                             s->payload.end());
            }
            else // replace-cpp-method
            {
                std::string cls;
                std::string method;

                if (!s->arg2.empty())
                {
                    cls = s->arg1;
                    method = s->arg2;
                }
                else
                {
                    auto pos = s->arg1.find("::");
                    if (pos == std::string::npos)
                        throw std::runtime_error(
                            "replace-cpp-method: expected 'Class::method' or "
                            "'Class method'");

                    cls = s->arg1.substr(0, pos);
                    method = s->arg1.substr(pos + 2);
                }

                if (cls.empty() || method.empty())
                    throw std::runtime_error(
                        "replace-cpp-method: empty class or method name");

                Region r;
                if (!finder.find_method(cls, method, r))
                    throw std::runtime_error(
                        "replace-cpp-method: method not found: " + cls +
                        "::" + method + " in file: " + filepath);

                if (r.start_line < 0 || r.end_line < r.start_line ||
                    r.end_line >= static_cast<int>(lines.size()))
                    throw std::runtime_error(
                        "replace-cpp-method: invalid region");

                auto begin = lines.begin() + r.start_line;
                auto end = lines.begin() + (r.end_line + 1);
                lines.erase(begin, end);
                lines.insert(lines.begin() + r.start_line,
                             s->payload.begin(),
                             s->payload.end());
            }

            continue;
        }

        if (s->command == "replace-py-class" ||
            s->command == "replace-py-method")
        {
            PythonSymbolFinder finder(text);

            if (s->command == "replace-py-class")
            {
                if (s->arg1.empty())
                    throw std::runtime_error(
                        "replace-py-class: missing class name for file: " +
                        filepath);

                Region r;
                if (!finder.find_class(s->arg1, r))
                    throw std::runtime_error(
                        "replace-py-class: class not found: " + s->arg1 +
                        " in file: " + filepath);

                if (r.start_line < 0 || r.end_line < r.start_line ||
                    r.end_line >= static_cast<int>(lines.size()))
                    throw std::runtime_error(
                        "replace-py-class: invalid region");

                auto begin = lines.begin() + r.start_line;
                auto end = lines.begin() + (r.end_line + 1);
                lines.erase(begin, end);
                lines.insert(lines.begin() + r.start_line,
                             s->payload.begin(),
                             s->payload.end());
            }
            else // replace-py-method
            {
                std::string cls;
                std::string method;

                if (!s->arg2.empty())
                {
                    cls = s->arg1;
                    method = s->arg2;
                }
                else
                {
                    auto pos = s->arg1.find('.');
                    if (pos == std::string::npos)
                        throw std::runtime_error(
                            "replace-py-method: expected 'Class.method' or "
                            "'Class method'");

                    cls = s->arg1.substr(0, pos);
                    method = s->arg1.substr(pos + 1);
                }

                if (cls.empty() || method.empty())
                    throw std::runtime_error(
                        "replace-py-method: empty class or method name");

                Region r;
                if (!finder.find_method(cls, method, r))
                    throw std::runtime_error(
                        "replace-py-method: method not found: " + cls + "." +
                        method + " in file: " + filepath);

                if (r.start_line < 0 || r.end_line < r.start_line ||
                    r.end_line >= static_cast<int>(lines.size()))
                    throw std::runtime_error(
                        "replace-py-method: invalid region");

                auto begin = lines.begin() + r.start_line;
                auto end = lines.begin() + (r.end_line + 1);
                lines.erase(begin, end);
                lines.insert(lines.begin() + r.start_line,
                             s->payload.begin(),
                             s->payload.end());
            }

            continue;
        }

        throw std::runtime_error("apply_symbol_commands: unknown command: " +
                                 s->command);
    }
}

static Section parse_section(std::istream &in, const std::string &header)
{
    Section s;

    auto pos = header.find(':');
    if (pos == std::string::npos)
        throw std::runtime_error("bad section header: " + header);

    auto pos2 = header.find("===", pos);
    if (pos2 == std::string::npos)
        pos2 = header.size();

    auto raw = header.substr(pos + 1, pos2 - pos - 1);
    s.filepath = trim(raw);
    if (s.filepath.empty())
        throw std::runtime_error("empty filepath in header: " + header);

    std::string line;
    if (!std::getline(in, line))
        throw std::runtime_error("unexpected end after header");

    if (line.rfind("--- ", 0) != 0)
        throw std::runtime_error("expected command after header");

    {
        std::istringstream ss(line.substr(4));
        ss >> s.command;

        // читаем остаток строки как аргументы команды
        std::string rest;
        std::getline(ss, rest);
        if (!rest.empty())
        {
            std::istringstream as(rest);
            as >> s.arg1;
            as >> s.arg2;
        }

        if (is_text_command(s.command))
        {
        }
        else if (s.command == "create-file" || s.command == "delete-file")
        {
        }
        else if (is_symbol_command(s.command))
        {
        }
        else
        {
            throw std::runtime_error("index-based commands removed: " +
                                     s.command);
        }
    }

    bool found_end = false;
    while (std::getline(in, line))
    {
        if (line == "=END=")
        {
            found_end = true;
            break;
        }
        s.payload.push_back(line);
    }

    if (!found_end)
        throw std::runtime_error("missing =END=");

    if (is_text_command(s.command))
    {
        // Определяем, в YAML-режиме мы или в старом формате.
        // Если сразу после команды нет BEFORE:/MARKER:/AFTER:, используется
        // старая логика.
        bool yaml_mode = false;
        std::size_t first_non_empty = 0;
        while (first_non_empty < s.payload.size() &&
               trim(s.payload[first_non_empty]).empty())
            ++first_non_empty;

        if (first_non_empty < s.payload.size())
        {
            const std::string t = trim(s.payload[first_non_empty]);
            if (t == "BEFORE:" || t == "MARKER:" || t == "AFTER:")
                yaml_mode = true;
        }

        if (!yaml_mode)
        {
            // Старый режим: всё до '---' — marker, после — payload
            auto it = std::find(
                s.payload.begin(), s.payload.end(), std::string("---"));
            if (it == s.payload.end())
                throw std::runtime_error(
                    "text command requires '---' separator");

            s.marker.assign(s.payload.begin(), it);

            std::vector<std::string> tail;
            if (std::next(it) != s.payload.end())
                tail.assign(std::next(it), s.payload.end());

            s.payload.swap(tail);

            if (s.marker.empty())
                throw std::runtime_error("empty text marker");
        }
        else
        {
            // YAML-подобный режим:
            // BEFORE:
            //   ...
            // MARKER:
            //   ...
            // AFTER:
            //   ...
            // ---
            // <payload>
            s.before.clear();
            s.marker.clear();
            s.after.clear();

            enum class Block
            {
                NONE,
                BEFORE,
                MARKER,
                AFTER
            };

            Block blk = Block::NONE;
            std::vector<std::string> new_payload;

            bool seen_separator = false;

            for (std::size_t i = first_non_empty; i < s.payload.size(); ++i)
            {
                const std::string &ln = s.payload[i];

                if (!seen_separator && ln == "---")
                {
                    seen_separator = true;
                    continue;
                }

                if (!seen_separator)
                {
                    const std::string t = trim(ln);
                    if (t == "BEFORE:")
                    {
                        blk = Block::BEFORE;
                        continue;
                    }
                    if (t == "MARKER:")
                    {
                        blk = Block::MARKER;
                        continue;
                    }
                    if (t == "AFTER:")
                    {
                        blk = Block::AFTER;
                        continue;
                    }

                    switch (blk)
                    {
                    case Block::BEFORE:
                        s.before.push_back(ln);
                        break;
                    case Block::MARKER:
                        s.marker.push_back(ln);
                        break;
                    case Block::AFTER:
                        s.after.push_back(ln);
                        break;
                    case Block::NONE:
                        throw std::runtime_error(
                            "unexpected content before YAML block tag");
                    }
                }
                else
                {
                    new_payload.push_back(ln);
                }
            }

            s.payload.swap(new_payload);

            if (s.marker.empty())
                throw std::runtime_error(
                    "YAML text command requires MARKER: section");
        }
    }

    return s;
}

static void apply_for_file(const std::string &filepath,
                           const std::vector<const Section *> &sections)
{
    fs::path p = filepath;
    std::vector<std::string> orig;
    bool existed = true;

    try
    {
        orig = read_file_lines(p);
    }
    catch (...)
    {
        existed = false;
        orig.clear();
    }

    for (const Section *s : sections)
    {
        if (!existed && s->command == "delete-file")
            throw std::runtime_error("delete-file: file does not exist");
    }

    for (const Section *s : sections)
    {
        if (s->command == "create-file")
        {
            write_file_lines(p, s->payload);
            return;
        }
        if (s->command == "delete-file")
        {
            std::error_code ec;
            fs::remove(p, ec);
            if (ec)
                throw std::runtime_error("delete-file failed");
            return;
        }
    }

    std::vector<const Section *> text_sections;
    std::vector<const Section *> symbol_sections;

    for (const Section *s : sections)
    {
        if (is_text_command(s->command))
            text_sections.push_back(s);
        else if (is_symbol_command(s->command))
            symbol_sections.push_back(s);
        else
            throw std::runtime_error("unexpected non-text command: " +
                                     s->command);
    }

    if (!text_sections.empty())
        apply_text_commands(filepath, orig, text_sections);

    if (!symbol_sections.empty())
        apply_symbol_commands(filepath, orig, symbol_sections);

    if (!text_sections.empty() || !symbol_sections.empty())
        write_file_lines(p, orig);
}

static void apply_all(const std::vector<Section> &sections)
{
    namespace fs = std::filesystem;

    // 1. Собираем список всех файлов, которые будут затронуты
    std::vector<std::string> files;
    files.reserve(sections.size());
    for (auto &s : sections)
        files.push_back(s.filepath);

    std::sort(files.begin(), files.end());
    files.erase(std::unique(files.begin(), files.end()), files.end());

    struct Backup
    {
        bool existed = false;
        std::vector<std::string> lines;
    };

    std::map<std::string, Backup> backup;

    // 2. Делаем резервную копию всех файлов
    for (auto &f : files)
    {
        Backup b;
        fs::path p = f;

        std::error_code ec;

        if (fs::exists(p, ec))
        {
            b.existed = true;

            try
            {
                b.lines = read_file_lines(p);
            }
            catch (...)
            {
                throw std::runtime_error("cannot read original file: " + f);
            }
        }
        else
        {
            b.existed = false;
        }

        backup[f] = std::move(b);
    }

    // 3. Применяем секции с защитой (try/catch)
    try
    {
        for (auto &s : sections)
        {
            std::vector<const Section *> single{&s};
            apply_for_file(s.filepath, single);
        }
    }
    catch (...)
    {
        // 4. Откат (rollback)
        for (auto &[path, b] : backup)
        {
            fs::path p = path;
            std::error_code ec;

            if (b.existed)
            {
                try
                {
                    write_file_lines(p, b.lines);
                }
                catch (...)
                {
                    // если даже откат не удался — сдаёмся
                }
            }
            else
            {
                fs::remove(p, ec);
            }
        }

        throw;
    }
}

int apply_chunk_main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "usage: apply_patch <patchfile>\n";
        return 1;
    }

    std::ifstream fin(argv[1], std::ios::binary);
    if (!fin)
    {
        std::cerr << "cannot open patch file: " << argv[1] << "\n";
        return 1;
    }

    std::ostringstream oss;
    oss << fin.rdbuf();
    std::string text = oss.str();

    // Находим первую непустую строку
    std::string first_nonempty;
    {
        std::istringstream iss(text);
        std::string line;
        while (std::getline(iss, line))
        {
            if (!trim(line).empty())
            {
                first_nonempty = line;
                break;
            }
        }
    }

    std::vector<Section> sections;

    try
    {
        int seq = 0;

        if (!first_nonempty.empty() &&
            first_nonempty.rfind("=== file:", 0) == 0)
        {
            // Старый формат патча
            std::istringstream iss(text);
            std::string line;
            while (std::getline(iss, line))
            {
                if (line.rfind("=== file:", 0) == 0)
                {
                    Section s = parse_section(iss, line);
                    s.seq = seq++;
                    sections.push_back(std::move(s));
                }
            }
        }
        else
        {
            // Новый YAML-формат: description + operations
            sections = parse_yaml_patch_text(text);
        }

        apply_all(sections);
    }
    catch (const std::exception &e)
    {
        std::cerr << "error while applying patch: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
