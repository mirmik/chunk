#pragma once

#include "command.h"
#include "command_parse_helpers.h"

class CreateFileCommand : public Command
{
public:
    CreateFileCommand() : Command("create-file") {}

    void parse(const nos::trent &tr) override
    {
        section_.filepath = command_parse::get_scalar(tr, "path");
        if (section_.filepath.empty())
            throw std::runtime_error("YAML patch: op 'create-file' requires 'path' key");

        section_.payload = command_parse::split_scalar_lines(
            command_parse::get_scalar(tr, "payload"));
        if (section_.payload.empty())
            throw std::runtime_error(
                "YAML patch: text op 'create-file' for file '" +
                section_.filepath + "' requires 'payload'");

        section_.comment = command_parse::get_scalar(tr, "comment");
    }

    void execute(std::vector<std::string> &lines) override
    {
        (void)lines;
    }
};

