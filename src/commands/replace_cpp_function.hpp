// #pragma once
// #include "command_base.hpp"
// #include "commands/symbol_utils.h"
// #include "languages/symbols.h"

// class ReplaceCppFunctionCommand : public RegionReplaceCommand
// {
// public:
//     ReplaceCppFunctionCommand()
//         : RegionReplaceCommand("replace-cpp-function")
//     {
//     }

//     void parse(const nos::trent &tr) override
//     {
//         RegionReplaceCommand::parse(tr);
//         std::string function_text = command_parse::get_scalar(tr, "function");
//         std::string symbol_text = command_parse::get_scalar(tr, "symbol");

//         if (!function_text.empty())
//         {
//             target_ = function_text;
//         }
//         else if (!symbol_text.empty())
//         {
//             target_ = symbol_text;
//         }
//         else
//         {
//             throw std::runtime_error(
//                 "YAML patch: op '" + command_name() +
//                 "' requires 'function' or 'symbol'");
//         }

//         if (target_.empty())
//         {
//             throw std::runtime_error(
//                 "replace-cpp-function: empty function name");
//         }
//     }

// protected:
//     bool find_region(const std::string &text, Region &r) const override
//     {
//         CppSymbolFinder finder(text);
//         return finder.find_function(target_, r);
//     }

//     std::string not_found_error() const override
//     {
//         return "replace-cpp-function: function not found: " + target_ +
//                " in file: " + filepath_;
//     }

//     std::string invalid_region_error() const override
//     {
//         return "replace-cpp-function: invalid region";
//     }

// private:
//     std::string target_;
// };
