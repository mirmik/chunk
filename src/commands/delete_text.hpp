#pragma once

#include "command_base.hpp"
#include "commands/text_utils.hpp"

class DeleteTextCommand : public MarkerTextCommand
{
public:
    DeleteTextCommand() : MarkerTextCommand("delete-text") {}

private:
    void apply(std::vector<std::string> &lines,
               std::size_t begin,
               std::size_t end,
               const std::vector<std::string> &payload) override
    {
        (void)payload;
        auto it_begin = lines.begin() + static_cast<std::ptrdiff_t>(begin);
        auto it_end = lines.begin() + static_cast<std::ptrdiff_t>(end + 1);
        lines.erase(it_begin, it_end);
    }
};

