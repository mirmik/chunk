#pragma once

#include <cstddef>
#include <string>
#include <vector>

// Диапазон строк в файле (0-based, включительно).
struct Region
{
    int start_line = -1; // inclusive
    int end_line = -1;   // inclusive
};

// Простейший поисковик C++-символов.
// Умеет:
//   * находить определение класса (class / struct)
//   * искать методы внутри класса (по имени)
class CppSymbolFinder
{
public:
    // text — полный текст файла.
    explicit CppSymbolFinder(const std::string &text);

    const std::vector<std::string> &lines() const
    {
        return m_lines;
    }

    // Находит определение класса (НЕ forward-declaration).
    // Возвращает true, если класс найден.
    bool find_class(const std::string &class_name, Region &out) const;

    // Находит метод внутри класса (по имени).
    // Ищет только внутри тела class/struct class_name.
    // Возвращает диапазон строк от начала объявления/определения
    // до ';' или закрывающей '}'.
    bool find_method(const std::string &class_name,
                     const std::string &method_name,
                     Region &out) const;

private:
    struct Token
    {
        enum Kind
        {
            Identifier,
            Keyword,
            Symbol
        } kind;
        std::string text;
        int line = 0; // 0-based
    };

    struct ClassRange
    {
        Region region;
        std::size_t body_start_token = 0; // индекс '{'
        std::size_t body_end_token = 0;   // индекс '}' (или '};')
    };

    std::vector<std::string> m_lines;
    std::vector<Token> m_tokens;

    void tokenize(const std::string &text);

    static bool is_ident_start(char c);
    static bool is_ident_char(char c);

    bool find_class_internal(const std::string &class_name,
                             ClassRange &out) const;
};

// Простейший поисковик Python-символов.
// Умеет:
//   * находить определение класса (class Foo:)
//   * искать методы внутри класса (def bar(self, ...))
class PythonSymbolFinder
{
public:
    explicit PythonSymbolFinder(const std::string &text);

    const std::vector<std::string> &lines() const
    {
        return m_lines;
    }

    // Находит определение класса (первое вхождение с таким именем).
    // Region покрывает строку 'class ...' и всё тело класса до dedent.
    bool find_class(const std::string &class_name, Region &out) const;

    // Находит метод внутри класса.
    // Ищет def / async def с именем method_name,
    // являющийся "первым уровнем" внутри тела class_name.
    // Region покрывает строку def (включая декораторы над ней) и всё тело до
    // dedent.
    bool find_method(const std::string &class_name,
                     const std::string &method_name,
                     Region &out) const;

private:
    std::vector<std::string> m_lines;

    static int calc_indent(const std::string &line);
    static std::size_t first_code_pos(const std::string &line);

    bool find_class_internal(const std::string &class_name,
                             Region &out,
                             int &class_indent) const;
};