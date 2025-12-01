#include "file_io.h"

#include <fstream>
#include <sstream>
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

std::string read_file_bytes(const std::filesystem::path &p)
{
    std::ifstream in(p, std::ios::binary);
    if (!in)
        throw std::runtime_error("cannot open file: " + p.string());

    std::string data;
    in.seekg(0, std::ios::end);
    std::streampos size = in.tellg();
    if (size > 0)
    {
        data.resize(static_cast<std::size_t>(size));
        in.seekg(0, std::ios::beg);
        in.read(&data[0], size);
    }

    return data;
}

void write_file_bytes(const std::filesystem::path &p, const std::string &data)
{
    std::ofstream out(p, std::ios::binary | std::ios::trunc);
    if (!out)
        throw std::runtime_error("cannot write file: " + p.string());

    if (!data.empty())
        out.write(data.data(), static_cast<std::streamsize>(data.size()));
}

std::string read_all(std::istream &in)
{
    std::ostringstream oss;
    oss << in.rdbuf();
    return oss.str();
}