#pragma once

#include "command_base.hpp"
#include "commands/symbol_utils.h"
#include "symbols.h"

class ReplaceCppClassCommand : public RegionReplaceCommand
{
public:
    ReplaceCppClassCommand() : RegionReplaceCommand("replace-cpp-class") {}

    void parse(const nos::trent &tr) override
    {
        RegionReplaceCommand::parse(tr);

        std::string class_text = command_parse::get_scalar(tr, "class");
        if (class_text.empty())
            throw std::runtime_error("YAML patch: op '" + command_name() +
                                     "' requires 'class' key");
        class_name_ = class_text;
    }

protected:
    bool find_region(const std::string &text, Region &r) const override
    {
        CppSymbolFinder finder(text);
        return finder.find_class(class_name_, r);
    }

    std::string not_found_error() const override
    {
        return "replace-cpp-class: class not found: " + class_name_ +
               " in file: " + filepath_;
    }

    std::string invalid_region_error() const override
    {
        return "replace-cpp-class: invalid region";
    }

private:
    std::string class_name_;
};

