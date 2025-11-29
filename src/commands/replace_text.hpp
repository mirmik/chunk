#pragma once

#include "command_base.hpp"
#include "commands/text_utils.h"

class ReplaceTextCommand : public MarkerTextCommand
{
public:
    ReplaceTextCommand() : MarkerTextCommand("replace-text") {}

protected:
    bool should_indent_payload() const override { return true; }

    void apply(std::vector<std::string> &lines,
               std::size_t begin,
               std::size_t end,
               const std::vector<std::string> &payload) override
    {
        auto it_begin = lines.begin() + static_cast<std::ptrdiff_t>(begin);
        auto it_end = lines.begin() + static_cast<std::ptrdiff_t>(end + 1);
        lines.erase(it_begin, it_end);
        lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(begin),
                     payload.begin(),
                     payload.end());
    }
};

