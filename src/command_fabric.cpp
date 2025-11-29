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

std::unique_ptr<Command> create_command(const Section &section,
                                        const std::string &filepath)
{
    const auto &registry = symbol_command_registry();
    auto it = registry.find(section.command);
    if (it == registry.end())
        throw std::runtime_error("apply_commands: unknown command: " +
                                 section.command);
    auto cmd = it->second();
    cmd->load_section(section);
    (void)filepath;
    return cmd;
}

std::unique_ptr<Command> make_command(const std::string &name)
{
    Section s;
    s.command = name;
    auto command = create_command(s, std::string());
    if (command)
        return command;

    throw std::runtime_error("YAML patch: unknown operation: " + name);
}
