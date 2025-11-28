#include "apply.h"

#include "runner.h"
#include "section.h"

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

    std::string read_all(std::istream &in)
    {
        std::ostringstream oss;
        oss << in.rdbuf();
        return oss.str();
    }

    struct ClipboardReader
    {
        const char *name;    // человекочитаемое имя
        const char *command; // команда для popen/_popen
        bool (*is_available)(); // опциональная проверка среды, nullptr если не
                                // нужна
    };

#ifdef _WIN32

    static const ClipboardReader kClipboardReaders[] = {
        {
            "powershell-GetClipboard",
            "powershell -NoProfile -Command \"[Console]::OutputEncoding = "
            "[System.Text.Encoding]::UTF8; Get-Clipboard -Raw\"",
            nullptr,
        },
    };

#else

    static bool has_wayland_monitor()
    {
        const char *env = std::getenv("WAYLAND_MONITOR");
        return env && *env;
    }

    static const ClipboardReader kClipboardReaders[] = {
        {"wl-paste", "wl-paste", &has_wayland_monitor},
        {"xclip", "xclip -selection clipboard -o", nullptr},
        {"xsel", "xsel -b", nullptr},
        {"pbpaste", "pbpaste", nullptr},
    };

#endif

    static bool run_clipboard_reader(const ClipboardReader &reader,
                                     std::string &out,
                                     bool verbose)
    {
        if (reader.is_available && !reader.is_available())
        {
            if (verbose)
            {
                std::cerr << "chunk: clipboard: skipping '" << reader.name
                          << "' (not available in current environment)\n";
            }
            return false;
        }

        if (verbose)
        {
            std::cerr << "chunk: clipboard: trying '" << reader.name << "'\n";
        }

#ifdef _WIN32
        FILE *pipe = _popen(reader.command, "r");
#else
        FILE *pipe = popen(reader.command, "r");
#endif
        if (!pipe)
        {
            if (verbose)
            {
                std::cerr << "chunk: clipboard: failed to start '"
                          << reader.name << "'\n";
            }
            return false;
        }

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

        if (rc != 0)
        {
            if (verbose)
            {
                std::cerr << "chunk: clipboard: '" << reader.name
                          << "' exited with code " << rc << "\n";
            }
            return false;
        }

        if (result.empty())
        {
            if (verbose)
            {
                std::cerr << "chunk: clipboard: '" << reader.name
                          << "' returned empty data\n";
            }
            return false;
        }

        out = std::move(result);

        if (verbose)
        {
            std::cerr << "chunk: clipboard: '" << reader.name << "' succeeded ("
                      << out.size() << " bytes)\n";
        }

        return true;
    }

    bool read_clipboard(std::string &out, bool verbose)
    {
        out.clear();

        const std::size_t count =
            sizeof(kClipboardReaders) / sizeof(kClipboardReaders[0]);

        if (count == 0)
        {
            if (verbose)
                std::cerr
                    << "chunk: clipboard: no clipboard readers configured\n";
            return false;
        }

        for (std::size_t i = 0; i < count; ++i)
        {
            const ClipboardReader &reader = kClipboardReaders[i];
            if (run_clipboard_reader(reader, out, verbose))
                return true;
        }

        if (verbose)
        {
            std::cerr << "chunk: clipboard: all clipboard commands failed\n";
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

    if ((use_stdin && use_clipboard) || (use_stdin && filename) ||
        (use_clipboard && filename))
    {
        std::cerr << "chunk: conflicting input options\n";
        return 1;
    }

    std::string text;

    if (use_stdin)
    {
        text = read_all(std::cin);
    }
    else if (use_clipboard)
    {
        if (!read_clipboard(text, verbose))
        {
            std::cerr << "cannot read patch from clipboard\n";
            return 1;
        }
    }
    else
    {
        if (!filename)
        {
            std::cerr
                << "chunk: missing input (patch file, --stdin or --paste)\n";
            return 1;
        }

        std::ifstream fin(filename, std::ios::binary);
        if (!fin)
        {
            std::cerr << "cannot open patch file: " << filename << "\n";
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
