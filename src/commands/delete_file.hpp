#pragma once

#include "command.h"
#include "command_parse_helpers.h"

class DeleteFileCommand : public Command
{
public:
    DeleteFileCommand() : Command("delete-file") {}

    void parse(const nos::trent &tr) override
    {
        filepath_ = command_parse::get_scalar(tr, "path");
        if (filepath_.empty())
            throw std::runtime_error(
                "YAML patch: op 'delete-file' requires 'path' key");
        comment_ = command_parse::get_scalar(tr, "comment");
    }

    void execute(std::vector<std::string> &lines) override
    {
        (void)lines;
    }
};

