#include "command.h"

Command::Command(std::string name)
{
    section_.command = std::move(name);
}
