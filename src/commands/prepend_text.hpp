#pragma once

#include "command_base.hpp"

class PrependTextCommand : public Command
{
public:
    PrependTextCommand() : Command("prepend-text") {}

    void parse(const nos::trent &tr) override
    {
        section_.filepath = command_parse::get_scalar(tr, "path");
        if (section_.filepath.empty())
            throw std::runtime_error("YAML patch: op 'prepend-text' requires 'path' key");

        section_.payload = command_parse::split_scalar_lines(
            command_parse::get_scalar(tr, "payload"));
        if (section_.payload.empty())
            throw std::runtime_error(
                "YAML patch: text op 'prepend-text' for file '" +
                section_.filepath + "' requires 'payload'");
        section_.comment = command_parse::get_scalar(tr, "comment");
    }

    void execute(std::vector<std::string> &lines) override
    {
        lines.insert(lines.begin(), section_.payload.begin(), section_.payload.end());
    }
};

