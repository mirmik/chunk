#include "apply.h"

#include "runner.h"
#include "parser.h"

#include "clipboard.h"
#include "file_io.h"
#include "command.h"
#include <map>
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

    void print_summary(const std::vector<std::unique_ptr<Command>> &commands,
                       bool dry_run)
    {
        std::map<std::string, std::vector<const Command *>> by_file;
        std::size_t total_inserted = 0;
        std::size_t total_removed = 0;
        std::size_t total_success = 0;
        std::size_t total_failed = 0;
        for (const auto &cmd_ptr : commands)
        {
            const Command *cmd = cmd_ptr.get();
            const std::string &path = cmd->filepath();
            const std::string key = path.empty() ? std::string("<no path>") : path;
            by_file[key].push_back(cmd);
            if (cmd->status() == Command::Status::Success)
            {
                ++total_success;
                total_inserted += cmd->chars_inserted();
                total_removed += cmd->chars_removed();
            }
            else if (cmd->status() == Command::Status::Failed)
            {
                ++total_failed;
            }
        }

        if (dry_run)
        {
            std::cout << "DRY-RAN summary (no changes were written to disk)\n";
        }
        else
        {
            std::cout << "Patch summary (changes were written to disk)\n";
        }

        for (const auto &entry : by_file)
        {
            const std::string &path = entry.first;
            const auto &list = entry.second;

            std::cout << "\nFile: " << path << "\n";
            for (const Command *cmd : list)
            {
                std::cout << "  - [";
                switch (cmd->status())
                {
                case Command::Status::Success:
                    std::cout << "OK";
                    break;
                case Command::Status::Failed:
                    std::cout << "FAIL";
                    break;
                default:
                    std::cout << "SKIP";
                    break;
                }

                std::cout << "] " << cmd->command_name();
                if (!cmd->comment().empty())
                    std::cout << "  # " << cmd->comment();
                if (cmd->status() == Command::Status::Success)
                {
                    std::size_t ins = cmd->chars_inserted();
                    std::size_t rem = cmd->chars_removed();
                    std::cout << " (+" << ins << " chars";
                    if (rem > 0)
                        std::cout << ", -" << rem << " chars";
                    std::cout << ")";
                }
                else if (cmd->status() == Command::Status::Failed)
                {
                    const std::string &msg = cmd->error_message();
                    if (!msg.empty())
                        std::cout << " (error: " << msg << ")";
                }
                std::cout << "\n";
            }
        }

        std::cout << "\nTotals: commands: " << commands.size()
                  << ", successful: " << total_success
                  << ", failed: " << total_failed
                  << ", chars inserted: " << total_inserted
                  << ", chars removed: " << total_removed
                  << "\n";
    }
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
                  << "  chunk --paste --verbose\n"
                  << "  chunk --dry-ran <patchfile>\n";
        return 1;
    }
    bool verbose = false;
    bool use_stdin = false;
    bool use_clipboard = false;
    bool dry_run = false;
    bool quiet = false;
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
        else if (std::strcmp(arg, "--dry-ran") == 0)
        {
            dry_run = true;
        }
        else if (std::strcmp(arg, "--quiet") == 0 ||
                 std::strcmp(arg, "--quite") == 0)
        {
            quiet = true;
        }
        else if (!filename)
        {
            filename = arg;
        }
    }
    bool effective_verbose = verbose && !quiet;
    g_chunk_verbose_logging = effective_verbose;
    if ((use_stdin && use_clipboard) || (use_stdin && filename) ||
        (use_clipboard && filename))
    {
        std::cerr << "chunk: conflicting input options\n";
        return 1;
    }

    auto [text, status] =
        get_script(use_stdin, use_clipboard, filename, effective_verbose);

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

    std::vector<std::unique_ptr<Command>> commands;
    try
    {
        commands = parse_yaml_patch_text(text);
        ApplyOptions options;
        options.dry_run = dry_run;
        options.ignore_failures = dry_run;
        apply_sections(commands, options);
    }
    catch (const std::exception &e)
    {
        if (!quiet && !commands.empty())
        {
            print_summary(commands, dry_run);
        }
        if (dry_run)
        {
            std::cerr << "chunk: dry-run aborted due to error: " << e.what()
                      << "\n";
        }
        else
        {
            std::cerr << "chunk: patch aborted due to error: " << e.what()
                      << "\n";
        }
        return 1;
    }

    if (!quiet)
    {
        print_summary(commands, dry_run);
    }

    return 0;
}
