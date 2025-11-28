#include "apply.h"
#include "help.h"
#include <cstring>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc >= 2 && (std::strcmp(argv[1], "--help") == 0 ||
                      std::strcmp(argv[1], "-h") == 0))
    {
        print_chunk_help();
        return 0;
    }

    if (argc >= 2 && (std::strcmp(argv[1], "--version") == 0 ||
                      std::strcmp(argv[1], "-V") == 0))
    {
        std::cout << "chunk 0.1.0\n";
        return 0;
    }

    if (argc >= 2 && (std::strcmp(argv[1], "--ref") == 0))
    {
        std::cout << "https://mirmik.github.io/chunk/\n";
        return 0;
    }

    return apply_chunk_main(argc, argv);
}
