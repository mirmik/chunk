#pragma once

#include "command.h"
#include "section.h"

#include <memory>
#include <string>
#include <vector>

void apply_symbol_commands(const std::string &filepath,
                           std::vector<std::string> &lines,
                           const std::vector<const Section *> &sections);
