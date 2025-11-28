#pragma once

#include <filesystem>
#include <string>
#include <vector>

std::vector<std::string> read_file_lines(const std::filesystem::path &p);
void write_file_lines(const std::filesystem::path &p,
                      const std::vector<std::string> &lines);

std::string read_file_bytes(const std::filesystem::path &p);
void write_file_bytes(const std::filesystem::path &p, const std::string &data);

std::string read_all(std::istream &in);