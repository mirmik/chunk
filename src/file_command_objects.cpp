#include "file_command_objects.h"

#include "command_parse_helpers.h"
#include "file_io.h"

#include <stdexcept>

using command_parse::get_scalar;
using command_parse::split_scalar_lines;

CreateFileCommand::CreateFileCommand() : Command("create-file") {}

void CreateFileCommand::parse(const nos::trent &tr)
{
    section_.filepath = get_scalar(tr, "path");
    if (section_.filepath.empty())
        throw std::runtime_error(
            "YAML patch: op 'create-file' requires 'path' key");

    section_.payload = split_scalar_lines(get_scalar(tr, "payload"));

    section_.comment = get_scalar(tr, "comment");
}

void CreateFileCommand::execute(std::vector<std::string> &lines)
{
    (void)lines;

    std::filesystem::path p(section_.filepath);
    if (std::filesystem::exists(p))
        return;
    write_file_lines(p, section_.payload);
}

DeleteFileCommand::DeleteFileCommand() : Command("delete-file") {}

void DeleteFileCommand::parse(const nos::trent &tr)
{
    section_.filepath = get_scalar(tr, "path");
    if (section_.filepath.empty())
        throw std::runtime_error(
            "YAML patch: op 'delete-file' requires 'path' key");
    section_.comment = get_scalar(tr, "comment");
}

void DeleteFileCommand::execute(std::vector<std::string> &lines)
{
    (void)lines;

    std::filesystem::path p(section_.filepath);
    if (!std::filesystem::exists(p))
        return;
    std::error_code ec;
    std::filesystem::remove(p, ec);
    if (ec)
        throw std::runtime_error("delete-file: cannot delete file: " +
                                 section_.filepath + ": " + ec.message());
}
