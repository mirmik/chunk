#pragma once

#include "section.h"

#include <string>
#include <vector>

void apply_text_commands(const std::string &filepath,
                         std::vector<std::string> &lines,
                         const std::vector<const Section *> &sections);
