#pragma once

#include "yaml/trent.h"

#include <string>
#include <vector>

namespace command_parse
{
    std::string get_scalar(const nos::trent &node, const char *key);
    std::vector<std::string> split_scalar_lines(const std::string &text);
    bool parse_indent_from_options(const nos::trent &node,
                                   bool default_value = true);
} // namespace command_parse
