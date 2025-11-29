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

std::size_t Command::count_total_chars(const std::vector<std::string> &lines)
{
    std::size_t total = 0;
    if (lines.empty())
        return 0;
    for (const auto &ln : lines)
        total += ln.size() + 1;
    return total;
}

void Command::record_effect(std::size_t before_size, std::size_t after_size)
{
    if (after_size >= before_size)
    {
        chars_inserted_ = after_size - before_size;
        chars_removed_ = 0;
    }
    else
    {
        chars_inserted_ = 0;
        chars_removed_ = before_size - after_size;
    }
}

void Command::run(std::vector<std::string> &lines)
{
    reset_status();
    chars_inserted_ = 0;
    chars_removed_ = 0;
    std::size_t before_size = count_total_chars(lines);
    try
    {
        execute(lines);
        std::size_t after_size = count_total_chars(lines);
        record_effect(before_size, after_size);
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
