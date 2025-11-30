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
        using text_utils::trim;

        std::size_t count = payload.size();
        const std::size_t marker_lines = marker_.size();
        if (marker_lines > 0 && count >= marker_lines)
        {
            bool same = true;
            const std::size_t start = count - marker_lines;
            for (std::size_t i = 0; i < marker_lines; ++i)
            {
                if (trim(marker_[i]) != trim(payload[start + i]))
                {
                    same = false;
                    break;
                }
            }
            if (same)
                count -= marker_lines;
        }

        auto insert_pos =
            lines.begin() + static_cast<std::ptrdiff_t>(begin);
        auto first = payload.begin();
        auto last =
            payload.begin() + static_cast<std::ptrdiff_t>(count);

        lines.insert(insert_pos, first, last);
    }
};

