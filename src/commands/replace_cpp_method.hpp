#pragma once

#include "command_base.hpp"
#include "commands/symbol_utils.h"
#include "symbols.h"

class ReplaceCppMethodCommand : public RegionReplaceCommand
{
public:
    ReplaceCppMethodCommand() : RegionReplaceCommand("replace-cpp-method") {}

    void parse(const nos::trent &tr) override
    {
        RegionReplaceCommand::parse(tr);

        std::string class_text = command_parse::get_scalar(tr, "class");
        std::string method_text = command_parse::get_scalar(tr, "method");
        std::string symbol_text = command_parse::get_scalar(tr, "symbol");

        if (!class_text.empty() && !method_text.empty())
        {
            cls = class_text;
            method = method_text;
        }
        else if (!symbol_text.empty())
        {
            auto pos = symbol_text.find("::");
            if (pos == std::string::npos)
                throw std::runtime_error(
                    "replace-cpp-method: expected 'Class::method' or 'Class method'");
            cls = symbol_text.substr(0, pos);
            method = symbol_text.substr(pos + 2);
        }
        else
        {
            throw std::runtime_error(
                "YAML patch: op '" + command_name() +
                "' requires 'class'+'method' or 'symbol'");
        }

        if (cls.empty() || method.empty())
            throw std::runtime_error(
                "replace-cpp-method: empty class or method name");
    }

protected:
    bool find_region(const std::string &text, Region &r) const override
    {
        CppSymbolFinder finder(text);
        return finder.find_method(cls, method, r);
    }

    std::string not_found_error() const override
    {
        return "replace-cpp-method: method not found: " + cls + "::" + method +
               " in file: " + filepath_;
    }

    std::string invalid_region_error() const override
    {
        return "replace-cpp-method: invalid region";
    }

private:
    std::string cls;
    std::string method;
};

