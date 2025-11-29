#pragma once

#include "command_base.hpp"
#include "commands/symbol_utils.h"
#include "symbols.h"

class ReplacePyClassCommand : public RegionReplaceCommand
{
public:
    ReplacePyClassCommand() : RegionReplaceCommand("replace-py-class") {}

    void parse(const nos::trent &tr) override
    {
        RegionReplaceCommand::parse(tr);

        std::string class_text = command_parse::get_scalar(tr, "class");
        if (class_text.empty())
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'class' key");
        section_.arg1 = class_text;
    }

protected:
    bool find_region(const std::string &text, Region &r) const override
    {
        PythonSymbolFinder finder(text);
        return finder.find_class(section_.arg1, r);
    }

    std::string not_found_error() const override
    {
        return "replace-py-class: class not found: " + section_.arg1 +
               " in file: " + section_.filepath;
    }

    std::string invalid_region_error() const override
    {
        return "replace-py-class: invalid region";
    }
};

