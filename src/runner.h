#pragma once

#include "command.h"

#include <vector>
#include <memory>

void apply_sections(const std::vector<std::unique_ptr<Command>> &commands);
