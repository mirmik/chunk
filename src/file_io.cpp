#include "file_io.h"

#include <fstream>
#include <stdexcept>

std::vector<std::string> read_file_lines(const std::filesystem::path &p)
{
    std::ifstream in(p);
    if (!in)
        throw std::runtime_error("cannot open file: " + p.string());

    std::vector<std::string> out;
    std::string line;
    while (std::getline(in, line))
        out.push_back(line);

    return out;
}

void write_file_lines(const std::filesystem::path &p,
                      const std::vector<std::string> &lines)
{
    std::ofstream out(p, std::ios::trunc);
    if (!out)
        throw std::runtime_error("cannot write file: " + p.string());

    for (const auto &s : lines)
        out << s << "\n";
}
