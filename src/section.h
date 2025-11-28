#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <vector>

struct Section
{
    std::string filepath;
    std::string command;
    std::string language;
    int a = -1;
    int b = -1;
    std::vector<std::string> payload;
    std::vector<std::string> marker;
    std::vector<std::string> before;
    std::vector<std::string> after;
    int seq = 0;
    std::string comment;
    std::string arg1;
    std::string arg2;
    bool indent_from_marker = true;
};

bool is_text_command(const std::string &cmd);
bool is_symbol_command(const std::string &cmd);
class Command;
std::vector<std::unique_ptr<Command>>
parse_yaml_patch_text(const std::string &text);
