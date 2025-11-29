#pragma once

#include "command_base.hpp"
#include "commands/text_utils.h"

class InsertBeforeTextCommand : public MarkerTextCommand
{
public:
    InsertBeforeTextCommand() : MarkerTextCommand("insert-before-text") {}

protected:
    bool should_indent_payload() const override { return true; }

    void apply(std::vector<std::string> &lines,
               std::size_t begin,
               std::size_t /*end*/,
               const std::vector<std::string> &payload) override
    {
        lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(begin),
                     payload.begin(),
                     payload.end());
    }
};

