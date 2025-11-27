#ifndef NOS_TRENT_YAML_H
#define NOS_TRENT_YAML_H

#include <iosfwd>
#include <nos/trent/trent.h>
#include <string>

namespace nos
{
    namespace yaml
    {
        nos::trent parse(const std::string &text);
        nos::trent parse(const char *text);
        nos::trent parse_file(const std::string &path);

        void print_to(const nos::trent &tr, std::ostream &os);
        std::string to_string(const nos::trent &tr);
    }
}

#endif
