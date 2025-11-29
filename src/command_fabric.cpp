#include "command_fabric.h"
#include "commands/append_text.hpp"
#include "commands/create_file.hpp"
#include "commands/delete_file.hpp"
#include "commands/delete_text.hpp"
#include "commands/insert_after_text.hpp"
#include "commands/insert_before_text.hpp"
#include "commands/prepend_text.hpp"
#include "commands/replace_cpp_class.hpp"
#include "commands/replace_cpp_method.hpp"
#include "commands/replace_cpp_function.hpp"
#include "commands/replace_py_class.hpp"
#include "commands/replace_py_method.hpp"
#include "commands/replace_text.hpp"
#include <functional>
#include <unordered_map>

namespace commands_detail
{
using CommandFactory = std::function<std::unique_ptr<Command>()>;
}

const std::unordered_map<std::string, commands_detail::CommandFactory> &
symbol_command_registry()
{
    using namespace commands_detail;

    static const std::unordered_map<std::string, CommandFactory> registry = {
        {"replace-cpp-class",
         []() { return std::make_unique<ReplaceCppClassCommand>(); }},
        {"replace-cpp-method",
         []() { return std::make_unique<ReplaceCppMethodCommand>(); }},
        {"replace-cpp-function",
         []() { return std::make_unique<ReplaceCppFunctionCommand>(); }},
        {"replace-py-class",
         []() { return std::make_unique<ReplacePyClassCommand>(); }},
        {"replace-py-method",
         []() { return std::make_unique<ReplacePyMethodCommand>(); }},

        {"prepend-text",
         []() { return std::make_unique<PrependTextCommand>(); }},
        {"append-text",
         []() { return std::make_unique<AppendTextCommand>(); }},
        {"insert-after-text",
         []() { return std::make_unique<InsertAfterTextCommand>(); }},
        {"insert-before-text",
         []() { return std::make_unique<InsertBeforeTextCommand>(); }},
        {"replace-text",
         []() { return std::make_unique<ReplaceTextCommand>(); }},
        {"delete-text", []() { return std::make_unique<DeleteTextCommand>(); }},

        {"create-file", []() { return std::make_unique<CreateFileCommand>(); }},
        {"delete-file", []() { return std::make_unique<DeleteFileCommand>(); }},
    };

    return registry;
}

std::unique_ptr<Command> make_command(const std::string &name)
{
    const auto &registry = symbol_command_registry();
    auto it = registry.find(name);
    if (it == registry.end())
        throw std::runtime_error("YAML patch: unknown operation: " + name);
    return it->second();
}
