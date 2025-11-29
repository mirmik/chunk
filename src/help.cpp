#include "help.h"
#include <iostream>

void print_chunk_help()
{
    std::cout
        << "chunk - apply chunk_v2 patches to a source tree\n"
        << "\n"
        << "Usage:\n"
        << "  chunk <patch.yml>\n"
        << "  chunk --stdin\n"
        << "  chunk --paste\n"
        << "  chunk --paste --verbose\n"
        << "  chunk --help | -h\n"
        << "  chunk --version\n"
        << "\n"
        << "Options:\n"
        << "  --stdin        Read patch from standard input.\n"
        << "  --paste        Read patch from system clipboard.\n"
        << "  --verbose      Print diagnostics while choosing clipboard tool.\n"
        << "  --help, -h     Show this help and exit.\n"
        << "  --version      Show version information and exit.\n"
        << "  --ref          Show links to patch format reference documentation.\n"
        << "\n"
        << "Patch format:\n"
        << "  Patches must be written in the chunk_v2 YAML format.\n"
        << "  See the specification:\n"
        << "    https://mirmik.github.io/chunk/CHUNK.html\n"
        << "\n"
        << "Clipboard notes:\n"
        << "  On Linux / macOS, chunk will try (in this order):\n"
        << "    wl-paste\n"
        << "    xclip -selection clipboard -o\n"
        << "    xsel -b\n"
        << "    pbpaste\n"
        << "  On Windows it uses:\n"
        << "    powershell -NoProfile -Command \"[Console]::OutputEncoding = "
           "[System.Text.Encoding]::UTF8; Get-Clipboard -Raw\"\n";
}
