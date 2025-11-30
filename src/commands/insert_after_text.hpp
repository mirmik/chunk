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
        using text_utils::trim;

        std::size_t header_skip = 0;
        const std::size_t marker_lines = marker_.size();
        if (marker_lines > 0 && payload.size() >= marker_lines)
        {
            bool same = true;
            for (std::size_t i = 0; i < marker_lines; ++i)
            {
                if (trim(marker_[i]) != trim(payload[i]))
                {
                    same = false;
                    break;
                }
            }
            if (same)
                header_skip = marker_lines;
        }

        (void)begin;
        std::size_t pos = end + 1;
        auto insert_pos =
            lines.begin() + static_cast<std::ptrdiff_t>(pos);
        auto first = payload.begin();
        if (header_skip > 0)
            first += static_cast<std::ptrdiff_t>(header_skip);

        lines.insert(insert_pos, first, payload.end());
    }
};

