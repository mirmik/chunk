#pragma once

#include "command_base.hpp"

class AppendTextCommand : public Command
{
public:
    AppendTextCommand() : Command("append-text") {}

    void parse(const nos::trent &tr) override
    {
        section_.filepath = command_parse::get_scalar(tr, "path");
        if (section_.filepath.empty())
            throw std::runtime_error("YAML patch: op 'append-text' requires 'path' key");

        section_.payload = command_parse::split_scalar_lines(
            command_parse::get_scalar(tr, "payload"));
        if (section_.payload.empty())
            throw std::runtime_error(
                "YAML patch: text op 'append-text' for file '" +
                section_.filepath + "' requires 'payload'");
        section_.comment = command_parse::get_scalar(tr, "comment");
    }

    void execute(std::vector<std::string> &lines) override
    {
        lines.insert(lines.end(), section_.payload.begin(), section_.payload.end());
    }
};

