#pragma once

#include "command_base.hpp"

class PrependTextCommand : public Command
{
public:
    PrependTextCommand() : Command("prepend-text") {}

    void parse(const nos::trent &tr) override
    {
        filepath_ = command_parse::get_scalar(tr, "path");
        if (filepath_.empty())
            throw std::runtime_error(
                "YAML patch: op 'prepend-text' requires 'path' key");

        std::string payload_text = command_parse::get_scalar(tr, "payload");
        payload_ = command_parse::split_scalar_lines(payload_text);
        if (payload_.empty())
            throw std::runtime_error(
                "YAML patch: text op 'prepend-text' for file '" +
                filepath_ + "' requires 'payload'");
        comment_ = command_parse::get_scalar(tr, "comment");
    }

    void execute(std::vector<std::string> &lines) override
    {
        lines.insert(lines.begin(), payload_.begin(), payload_.end());
    }

private:
    std::vector<std::string> payload_;
};

