#pragma once

#include "command.h"
#include "section.h"

#include <string>
#include <memory>
#include <vector>

void apply_text_commands(const std::string &filepath,
                         std::vector<std::string> &lines,
                         const std::vector<const Section *> &sections);

std::unique_ptr<Command> create_text_command(const Section &section,
                                             const std::string &filepath);
