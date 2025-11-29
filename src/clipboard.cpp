#include "clipboard.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>

namespace
{

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

} // namespace

bool run_clipboard_reader(const ClipboardReader &reader,
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
            std::cerr << "chunk: clipboard: failed to start '" << reader.name
                      << "'\n";
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
            std::cerr << "chunk: clipboard: no clipboard readers configured\n";
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
