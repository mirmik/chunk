#pragma once

#include <string>
#include <vector>

class Command
{
public:
    virtual ~Command() = default;
    virtual void execute(std::vector<std::string> &lines) = 0;
};

