#include "file_command_objects.h"

#include "command_parse_helpers.h"
#include "file_io.h"

#include <stdexcept>

using command_parse::get_scalar;
using command_parse::split_scalar_lines;

CreateFileCommand::CreateFileCommand()
    : Command("create-file")
{
}

void CreateFileCommand::parse(const nos::trent &tr)
{
    section_.filepath = get_scalar(tr, "path");
    if (section_.filepath.empty())
        throw std::runtime_error("YAML patch: op 'create-file' requires 'path' key");

    section_.payload = split_scalar_lines(get_scalar(tr, "payload"));

    section_.comment = get_scalar(tr, "comment");
}

void CreateFileCommand::execute(std::vector<std::string> &lines)
{
    (void)lines;
}

DeleteFileCommand::DeleteFileCommand()
    : Command("delete-file")
{
}

void DeleteFileCommand::parse(const nos::trent &tr)
{
    section_.filepath = get_scalar(tr, "path");
    if (section_.filepath.empty())
        throw std::runtime_error("YAML patch: op 'delete-file' requires 'path' key");
    section_.comment = get_scalar(tr, "comment");
}

void DeleteFileCommand::execute(std::vector<std::string> &lines)
{
    (void)lines;
}

std::unique_ptr<Command> create_file_command(const std::string &name)
{
    if (name == "create-file")
        return std::make_unique<CreateFileCommand>();
    if (name == "delete-file")
        return std::make_unique<DeleteFileCommand>();
    return nullptr;
}
