#include "command.h"

#include <ostream>

Command::Command(std::string name)
    : name_(std::move(name))
{
}

void Command::append_debug_info(std::ostream &) const
{
}
