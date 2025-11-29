#pragma once

#include "command_base.hpp"
#include "commands/text_utils.h"

class InsertAfterTextCommand : public MarkerTextCommand
{
public:
    InsertAfterTextCommand() : MarkerTextCommand("insert-after-text") {}

protected:
    bool should_indent_payload() const override { return true; }

    void apply(std::vector<std::string> &lines,
               std::size_t begin,
               std::size_t end,
               const std::vector<std::string> &payload) override
    {
        (void)begin;
        std::size_t pos = end + 1;
        lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(pos),
                     payload.begin(),
                     payload.end());
    }
};

