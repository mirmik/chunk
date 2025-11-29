#include "command.h"
#include <exception>

#include <ostream>

Command::Command(std::string name)
    : name_(std::move(name))
{
}

void Command::reset_status()
{
    status_ = Status::NotRun;
    error_message_.clear();
}

void Command::mark_success()
{
    status_ = Status::Success;
    error_message_.clear();
}

void Command::mark_failed(const std::string &message)
{
    status_ = Status::Failed;
    error_message_ = message;
}

void Command::run(std::vector<std::string> &lines)
{
    reset_status();
    try
    {
        execute(lines);
        mark_success();
    }
    catch (const std::exception &e)
    {
        mark_failed(e.what());
    }
    catch (...)
    {
        mark_failed("unknown error while executing command");
    }
}
void Command::append_debug_info(std::ostream &) const
{
}
