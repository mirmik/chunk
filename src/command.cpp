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

void Command::record_effect(const std::vector<std::string> &before,
                            const std::vector<std::string> &after)
{
    // сначала считаем байты, как и раньше
    std::size_t before_size = count_total_chars(before);
    std::size_t after_size = count_total_chars(after);
    record_effect(before_size, after_size);

    // затем считаем изменение по строкам и диапазон
    lines_inserted_ = 0;
    lines_removed_ = 0;
    has_effect_region_ = false;
    effect_start_line_ = 0;
    effect_end_line_ = 0;

    const std::size_t before_lines = before.size();
    const std::size_t after_lines = after.size();

    bool equal = (before_lines == after_lines);
    if (equal)
    {
        for (std::size_t i = 0; i < before_lines; ++i)
        {
            if (before[i] != after[i])
            {
                equal = false;
                break;
            }
        }
    }
    if (equal)
    {
        return;
    }

    std::size_t prefix = 0;
    while (prefix < before_lines && prefix < after_lines &&
           before[prefix] == after[prefix])
    {
        ++prefix;
    }

    std::size_t suffix = 0;
    while (suffix < before_lines - prefix &&
           suffix < after_lines - prefix &&
           before[before_lines - 1 - suffix] == after[after_lines - 1 - suffix])
    {
        ++suffix;
    }

    std::size_t before_changed_start = prefix;
    std::size_t before_changed_end = before_lines - suffix;
    std::size_t after_changed_start = prefix;
    std::size_t after_changed_end = after_lines - suffix;

    if (before_changed_start > before_changed_end)
        before_changed_start = before_changed_end;
    if (after_changed_start > after_changed_end)
        after_changed_start = after_changed_end;

    if (before_changed_end > before_changed_start)
        lines_removed_ = before_changed_end - before_changed_start;
    else
        lines_removed_ = 0;

    if (after_changed_end > after_changed_start)
        lines_inserted_ = after_changed_end - after_changed_start;
    else
        lines_inserted_ = 0;

    if (lines_inserted_ == 0 && lines_removed_ == 0)
    {
        return;
    }

    has_effect_region_ = true;

    if (lines_removed_ > 0)
    {
        effect_start_line_ = before_changed_start + 1;
        effect_end_line_ = before_changed_end;
        if (effect_end_line_ < effect_start_line_)
            effect_end_line_ = effect_start_line_;
    }
    else
    {
        effect_start_line_ = after_changed_start + 1;
        effect_end_line_ = effect_start_line_;
    }
}

void Command::run(std::vector<std::string> &lines)
{
    reset_status();
    chars_inserted_ = 0;
    chars_removed_ = 0;
    lines_inserted_ = 0;
    lines_removed_ = 0;
    has_effect_region_ = false;
    effect_start_line_ = 0;
    effect_end_line_ = 0;

    std::vector<std::string> before = lines;

    try
    {
        execute(lines);
        record_effect(before, lines);
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
