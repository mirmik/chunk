#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <vector>

class Command;
std::vector<std::unique_ptr<Command>> parse_yaml_patch_text(const std::string &text);
