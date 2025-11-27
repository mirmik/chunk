#include "apply.h"

#include "runner.h"
#include "section.h"

#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{

std::string read_all(std::istream &in)
{
    std::ostringstream oss;
    oss << in.rdbuf();
    return oss.str();
}

bool read_clipboard(std::string &out)
{
#ifdef _WIN32
    const char *commands[] = {
        "powershell -command Get-Clipboard",
    };
#else
    const char *commands[] = {
        "wl-paste",
        "xclip -selection clipboard -o",
        "xsel -b",
        "pbpaste",
    };
#endif

    for (const char *cmd : commands)
    {
        if (!cmd || !*cmd)
            continue;

#ifdef _WIN32
        FILE *pipe = _popen(cmd, "r");
#else
        FILE *pipe = popen(cmd, "r");
#endif
        if (!pipe)
            continue;

        std::string result;
        char buffer[4096];

        while (true)
        {
            std::size_t n = std::fread(buffer, 1, sizeof(buffer), pipe);
            if (n == 0)
                break;
            result.append(buffer, n);
        }

#ifdef _WIN32
        int rc = _pclose(pipe);
#else
        int rc = pclose(pipe);
#endif

        if (rc == 0 && !result.empty())
        {
            out = std::move(result);
            return true;
        }
    }

    return false;
}

} // namespace

int apply_chunk_main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage:\n"
                  << "  chunk <patchfile>\n"
                  << "  chunk --stdin\n"
                  << "  chunk --paste\n";
        return 1;
    }

    std::string text;

    if (std::strcmp(argv[1], "--stdin") == 0)
    {
        text = read_all(std::cin);
    }
    else if (std::strcmp(argv[1], "--paste") == 0)
    {
        if (!read_clipboard(text))
        {
#ifdef _WIN32
            std::cerr << "cannot read patch from clipboard "
                         "(tried: powershell Get-Clipboard)\n";
#else
            std::cerr << "cannot read patch from clipboard "
                         "(tried: wl-paste, xclip, xsel, pbpaste)\n";
#endif
            return 1;
        }
    }
    else
    {
        std::ifstream fin(argv[1], std::ios::binary);
        if (!fin)
        {
            std::cerr << "cannot open patch file: " << argv[1] << "\n";
            return 1;
        }

        text = read_all(fin);
    }

    if (text.empty())
    {
        std::cerr << "empty patch input\n";
        return 1;
    }

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
