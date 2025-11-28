#pragma once

#include "command.h"

#include <memory>

class CreateFileCommand : public Command
{
public:
    CreateFileCommand();
    void parse(const nos::trent &tr) override;
    void execute(std::vector<std::string> &lines) override;
};

class DeleteFileCommand : public Command
{
public:
    DeleteFileCommand();
    void parse(const nos::trent &tr) override;
    void execute(std::vector<std::string> &lines) override;
};

std::unique_ptr<Command> create_file_command(const std::string &name);
