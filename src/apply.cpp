#include "apply.h"

#include "runner.h"
#include "parser.h"

#include "clipboard.h"
#include "file_io.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
namespace
{
    bool g_chunk_verbose_logging = false;
}

bool chunk_verbose_logging_enabled()
{
    return g_chunk_verbose_logging;
}

std::tuple<std::string, std::string> get_script(bool use_stdin,
                                                bool use_clipboard,
                                                const char *filename,
                                                bool verbose)
{
    std::string text;
    if (use_stdin)
    {
        text = read_all(std::cin);
    }
    else if (use_clipboard)
    {
        if (!read_clipboard(text, verbose))
        {
            return {"", "cannot read patch from clipboard\n"};
        }
    }
    else
    {
        if (!filename)
        {
            return {"", "missing input (patch file, --stdin or --paste)\n"};
        }

        std::ifstream fin(filename, std::ios::binary);
        if (!fin)
        {
            return {"",
                    "cannot open patch file: " + std::string(filename) + "\n"};
        }

        text = read_all(fin);
    }
    return {text, ""};
}

int apply_chunk_main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage:\n"
                  << "  chunk <patchfile>\n"
                  << "  chunk --stdin\n"
                  << "  chunk --paste\n"
                  << "  chunk --paste --verbose\n";
        return 1;
    }

    bool verbose = false;
    bool use_stdin = false;
    bool use_clipboard = false;
    const char *filename = nullptr;

    for (int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];
        if (std::strcmp(arg, "--verbose") == 0)
        {
            verbose = true;
        }
        else if (std::strcmp(arg, "--stdin") == 0)
        {
            use_stdin = true;
        }
        else if (std::strcmp(arg, "--paste") == 0)
        {
            use_clipboard = true;
        }
        else if (!filename)
        {
            filename = arg;
        }
    }

    g_chunk_verbose_logging = verbose;
    if ((use_stdin && use_clipboard) || (use_stdin && filename) ||
        (use_clipboard && filename))
    {
        std::cerr << "chunk: conflicting input options\n";
        return 1;
    }

    auto [text, status] =
        get_script(use_stdin, use_clipboard, filename, verbose);

    if (!status.empty())
    {
        std::cerr << status;
        return 1;
    }

    if (text.empty())
    {
        std::cerr << "empty patch input\n";
        return 1;
    }

    try
    {
        auto commands = parse_yaml_patch_text(text);
        apply_sections(commands);
    }
    catch (const std::exception &e)
    {
        std::cerr << "error while applying patch: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
