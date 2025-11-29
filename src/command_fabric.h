#pragma once

#include "command.h"
#include "section.h"

#include <memory>
#include <string>

std::unique_ptr<Command> make_command(const std::string &name);