#include "apply.h"
#include "help.h"
#include <cstring>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc >= 2 &&
        (std::strcmp(argv[1], "--help") == 0 ||
         std::strcmp(argv[1], "-h") == 0))
    {
        print_chunk_help();
        return 0;
    }

    return apply_chunk_main(argc, argv);
}
