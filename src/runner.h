#pragma once

#include "command.h"

#include <memory>
#include <vector>

struct ApplyOptions
{
    bool dry_run = false;
    bool ignore_failures = false;
};

void apply_sections(const std::vector<std::unique_ptr<Command>> &commands,
                    const ApplyOptions &options = {});
