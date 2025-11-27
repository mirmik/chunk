#include "apply.h"

#include "runner.h"
#include "section.h"

#include <fstream>
#include <iostream>
#include <sstream>

int apply_chunk_main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: chunk <patchfile>\n";
        return 1;
    }

    std::ifstream fin(argv[1], std::ios::binary);
    if (!fin)
    {
        std::cerr << "cannot open patch file: " << argv[1] << "\n";
        return 1;
    }

    std::ostringstream oss;
    oss << fin.rdbuf();
    std::string text = oss.str();

    try
    {
        auto sections = parse_yaml_patch_text(text);
        apply_sections(sections);
    }
    catch (const std::exception &e)
    {
        std::cerr << "error while applying patch: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
