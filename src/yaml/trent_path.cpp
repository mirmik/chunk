#include "trent_path.h"


std::string nos::to_string(const nos::trent_path &path)
{
    std::vector<std::string> svec;
    for (const auto &node : path)
    {
        if (node.is_string)
            svec.push_back(node.str);
        else
            svec.push_back(std::to_string(node.i32));
    }
    return nos::join(svec, '/');
}