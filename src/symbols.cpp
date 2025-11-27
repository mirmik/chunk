#include "symbols.h"

#include <cctype>

CppSymbolFinder::CppSymbolFinder(const std::string &text)
{
    // Разбиваем текст на строки (независимо от \r\n / \n)
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

    tokenize(text);
}

bool CppSymbolFinder::is_ident_start(char c)
{
    unsigned char uc = static_cast<unsigned char>(c);
    return std::isalpha(uc) || c == '_';
}

bool CppSymbolFinder::is_ident_char(char c)
{
    unsigned char uc = static_cast<unsigned char>(c);
    return std::isalnum(uc) || c == '_';
}

void CppSymbolFinder::tokenize(const std::string &text)
{
    m_tokens.clear();

    bool in_block_comment = false;
    bool in_string = false;
    bool in_char = false;

    int line = 0;
    const std::size_t n = text.size();

    for (std::size_t i = 0; i < n;)
    {
        char c = text[i];

        if (c == '\n')
        {
            ++line;
            ++i;
            continue;
        }

        if (in_block_comment)
        {
            if (c == '*' && i + 1 < n && text[i + 1] == '/')
            {
                in_block_comment = false;
                i += 2;
            }
            else
            {
                ++i;
            }
            continue;
        }

        if (in_string || in_char)
        {
            if (c == '\\' && i + 1 < n)
            {
                i += 2; // экранированный символ
                continue;
            }
            if (in_string && c == '"')
            {
                in_string = false;
            }
            else if (in_char && c == '\'')
            {
                in_char = false;
            }
            ++i;
            continue;
        }

        // Пробелы
        if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v')
        {
            ++i;
            continue;
        }

        // Препроцессорная директива: пропускаем всю строку
        if (c == '#' && (i == 0 || text[i - 1] == '\n'))
        {
            while (i < n && text[i] != '\n')
                ++i;
            continue;
        }

        // Однострочный комментарий //
        if (c == '/' && i + 1 < n && text[i + 1] == '/')
        {
            while (i < n && text[i] != '\n')
                ++i;
            continue;
        }

        // Блочный комментарий /* ... */
        if (c == '/' && i + 1 < n && text[i + 1] == '*')
        {
            in_block_comment = true;
            i += 2;
            continue;
        }

        // Строковый литерал
        if (c == '"')
        {
            in_string = true;
            ++i;
            continue;
        }

        // Символьный литерал
        if (c == '\'')
        {
            in_char = true;
            ++i;
            continue;
        }

        // Идентификатор / ключевое слово
        if (is_ident_start(c))
        {
            std::size_t start = i;
            ++i;
            while (i < n && is_ident_char(text[i]))
                ++i;

            std::string ident = text.substr(start, i - start);

            Token tok;
            tok.text = ident;
            tok.line = line;
            if (ident == "class" || ident == "struct")
                tok.kind = Token::Keyword;
            else
                tok.kind = Token::Identifier;

            m_tokens.push_back(std::move(tok));
            continue;
        }

        // Двойное двоеточие ::
        if (c == ':' && i + 1 < n && text[i + 1] == ':')
        {
            Token tok;
            tok.kind = Token::Symbol;
            tok.text = "::";
            tok.line = line;
            m_tokens.push_back(std::move(tok));
            i += 2;
            continue;
        }

        // Любой пунктуационный символ как отдельный токен
        if (std::ispunct(static_cast<unsigned char>(c)))
        {
            Token tok;
            tok.kind = Token::Symbol;
            tok.text = std::string(1, c);
            tok.line = line;
            m_tokens.push_back(std::move(tok));
            ++i;
            continue;
        }

        // На всякий случай: пропускаем остальные символы
        ++i;
    }
}

bool CppSymbolFinder::find_class_internal(const std::string &class_name,
                                          ClassRange &out) const
{
    const std::size_t n = m_tokens.size();

    for (std::size_t i = 0; i < n; ++i)
    {
        const Token &t = m_tokens[i];
        if (t.kind != Token::Keyword)
            continue;
        if (t.text != "class" && t.text != "struct")
            continue;

        // Ищем первое имя после class/struct
        std::size_t j = i + 1;
        while (j < n && m_tokens[j].kind != Token::Identifier)
            ++j;
        if (j >= n)
            break;

        const Token &name_tok = m_tokens[j];
        if (name_tok.text != class_name)
            continue;

        // Определение или forward-declaration?
        // Смотрим только до первой ';' или '{' после имени.
        std::size_t k = j + 1;
        bool saw_lbrace = false;
        for (; k < n; ++k)
        {
            const Token &tk = m_tokens[k];
            if (tk.text == "{")
            {
                saw_lbrace = true;
                break;
            }
            if (tk.text == ";")
            {
                // forward-decl: встретили ';' раньше '{'
                break;
            }
        }

        // Если до ';' так и не встретили '{', это forward-declaration.
        if (!saw_lbrace)
            continue;

        // Ищем соответствующую закрывающую '}'
        int depth = 1;
        std::size_t body_start = k;
        std::size_t body_end = k;

        for (std::size_t p = k + 1; p < n; ++p)
        {
            const Token &tp = m_tokens[p];
            if (tp.text == "{")
            {
                ++depth;
            }
            else if (tp.text == "}")
            {
                --depth;
                if (depth == 0)
                {
                    body_end = p;

                    // Захватываем возможное ';' после '}' (class Foo {...};)
                    std::size_t q = p + 1;
                    if (q < n && m_tokens[q].text == ";")
                        body_end = q;

                    out.region.start_line = t.line;
                    out.region.end_line = m_tokens[body_end].line;
                    out.body_start_token = body_start;
                    out.body_end_token = body_end;
                    return true;
                }
            }
        }
    }

    return false;
}

bool CppSymbolFinder::find_class(const std::string &class_name,
                                 Region &out) const
{
    ClassRange cr;
    if (!find_class_internal(class_name, cr))
        return false;

    out = cr.region;
    return true;
}

bool CppSymbolFinder::find_method(const std::string &class_name,
                                  const std::string &method_name,
                                  Region &out) const
{
    ClassRange cr;
    if (!find_class_internal(class_name, cr))
        return false;

    const std::size_t begin = cr.body_start_token + 1;
    const std::size_t end = cr.body_end_token;
    const std::size_t n = m_tokens.size();

    if (begin >= n || begin >= end)
        return false;

    // Ищем внутри тела класса
    for (std::size_t i = begin; i < end; ++i)
    {
        const Token &t = m_tokens[i];
        if (t.kind != Token::Identifier)
            continue;
        if (t.text != method_name)
            continue;

        // После имени должен идти '('
        std::size_t j = i + 1;
        if (j >= end || m_tokens[j].text != "(")
            continue;

        // Определяем начало объявления:
        // идём назад до ближайшего ';', '{' или '}'.
        std::size_t start_tok = begin;
        for (std::size_t k = i; k > begin; --k)
        {
            const std::string &s = m_tokens[k].text;
            if (s == ";" || s == "{" || s == "}")
            {
                start_tok = k + 1;
                break;
            }
        }

        // 1) Пропускаем параметры (балансируем скобки)
        std::size_t pos = j;
        int paren_depth = 0;
        for (; pos < end; ++pos)
        {
            const std::string &s = m_tokens[pos].text;
            if (s == "(")
            {
                ++paren_depth;
            }
            else if (s == ")")
            {
                --paren_depth;
                if (paren_depth == 0)
                {
                    ++pos; // идём на токен после ')'
                    break;
                }
            }
        }
        if (paren_depth != 0)
            continue; // сломанная сигнатура

        // 2) Ищем ';' или '{' (начало тела)
        std::size_t end_tok = start_tok;
        int brace_depth = 0;

        for (; pos < end; ++pos)
        {
            const std::string &s = m_tokens[pos].text;

            if (s == ";")
            {
                end_tok = pos;
                break;
            }

            if (s == "{")
            {
                brace_depth = 1;
                end_tok = pos;
                ++pos;
                for (; pos < end; ++pos)
                {
                    const std::string &sb = m_tokens[pos].text;
                    if (sb == "{")
                        ++brace_depth;
                    else if (sb == "}")
                    {
                        --brace_depth;
                        if (brace_depth == 0)
                        {
                            end_tok = pos;
                            break;
                        }
                    }
                }
                break;
            }
        }

        if (end_tok < start_tok)
            continue;

        int start_line_token = m_tokens[start_tok].line;
        int method_line = t.line;

        // Не вываливаемся на строку с access-specifier'ом (public:)
        out.start_line =
            (start_line_token < method_line) ? method_line : start_line_token;

        out.end_line = m_tokens[end_tok].line;
        return true;
    }

    return false;
}
