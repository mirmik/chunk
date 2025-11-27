#ifndef NOS_TRENT_YAML_H
#define NOS_TRENT_YAML_H

#include <iosfwd>
#include "trent.h"
#include <string>
#include <stdexcept>

namespace nos
{
    namespace yaml
    {
        class parse_error : public std::runtime_error
        {
            size_t line_ = 0;
            size_t column_ = 0;

        public:
            parse_error(size_t line, size_t column, const std::string &msg);
            size_t line() const noexcept { return line_; }
            size_t column() const noexcept { return column_; }
        };

        nos::trent parse(const std::string &text);
        nos::trent parse(const char *text);
        nos::trent parse_file(const std::string &path);

        void print_to(const nos::trent &tr, std::ostream &os);
        std::string to_string(const nos::trent &tr);
    }
}

#endif
