#pragma once

#include <string>
#include <vector>

std::string extract_indent_prefix(const std::vector<std::string> &lines,
                                         int index);

std::vector<std::string>
apply_indent_prefix(const std::vector<std::string> &payload,
                    const std::string &prefix,
                    bool enabled);

std::string join_lines(const std::vector<std::string> &lines);

