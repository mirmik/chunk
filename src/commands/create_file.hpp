#pragma once

#include "command.h"
#include "command_parse_helpers.h"

class CreateFileCommand : public Command
{
public:
    CreateFileCommand() : Command("create-file") {}

    void parse(const nos::trent &tr) override
    {
        filepath_ = command_parse::get_scalar(tr, "path");
        if (filepath_.empty())
            throw std::runtime_error(
                "YAML patch: op 'create-file' requires 'path' key");

        std::string payload_text = command_parse::get_scalar(tr, "payload");
        payload_ = command_parse::split_scalar_lines(payload_text);
        if (payload_.empty())
            throw std::runtime_error(
                "YAML patch: text op 'create-file' for file '" +
                filepath_ + "' requires 'payload'");

        comment_ = command_parse::get_scalar(tr, "comment");
    }

    void execute(std::vector<std::string> &lines) override
    {
        lines = payload_;
    }

private:
    std::vector<std::string> payload_;
};

