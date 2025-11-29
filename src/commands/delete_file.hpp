#pragma once

#include "command.h"
#include "command_parse_helpers.h"

class DeleteFileCommand : public Command
{
public:
    DeleteFileCommand() : Command("delete-file") {}

    void parse(const nos::trent &tr) override
    {
        section_.filepath = command_parse::get_scalar(tr, "path");
        if (section_.filepath.empty())
            throw std::runtime_error("YAML patch: op 'delete-file' requires 'path' key");
        section_.comment = command_parse::get_scalar(tr, "comment");
    }

    void execute(std::vector<std::string> &lines) override
    {
        (void)lines;
    }
};

