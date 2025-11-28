#pragma once

#include "command.h"

#include <memory>
#include <vector>

void apply_sections(const std::vector<std::unique_ptr<Command>> &commands);
