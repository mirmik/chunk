#include "runner.h"

#include "command.h"
#include "file_io.h"
#include "symbol_commands.h"
#include "text_commands.h"
#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace
{
    struct Backup
    {
        bool existed = false;
        std::string data;
        fs::file_time_type last_write_time;
        fs::perms permissions = fs::perms::unknown;
        bool has_last_write_time = false;
        bool has_permissions = false;
    };

    std::string restore_backup(const std::string &path, const Backup &b)
    {
        fs::path p = path;
        std::error_code ec;

        if (!b.existed)
        {
            fs::remove(p, ec);
            if (ec)
                return "rollback: failed to remove file '" + path +
                       "': " + ec.message();
            return {};
        }

        try
        {
            write_file_bytes(p, b.data);
        }
        catch (const std::exception &e)
        {
            return "rollback: failed to restore data for file '" + path +
                   "': " + e.what();
        }
        catch (...)
        {
            return "rollback: failed to restore data for file '" + path +
                   "': unknown error";
        }

        if (b.has_permissions)
        {
            std::error_code perm_ec;
            fs::permissions(
                p, b.permissions, fs::perm_options::replace, perm_ec);
            if (perm_ec)
            {
                return "rollback: failed to restore permissions for file '" +
                       path + "': " + perm_ec.message();
            }
        }

        if (b.has_last_write_time)
        {
            std::error_code time_ec;
            fs::last_write_time(p, b.last_write_time, time_ec);
            if (time_ec)
            {
                return "rollback: failed to restore timestamp for file '" +
                       path + "': " + time_ec.message();
            }
        }

        return {};
    }

    void apply_for_file(const std::string &filepath,
                        const std::vector<Command *> &commands)
    {
        fs::path p = filepath;
        std::vector<std::string> orig;
        bool existed = true;

        try
        {
            orig = read_file_lines(p);
        }
        catch (...)
        {
            existed = false;
            orig.clear();
        }

        for (Command *cmd : commands)
        {
            if (!existed && cmd->command_name() == "delete-file")
                throw std::runtime_error("delete-file: file does not exist");
        }

        for (Command *cmd : commands)
        {
            if (cmd->command_name() == "create-file")
            {
                write_file_lines(p, cmd->data().payload);
                return;
            }
            if (cmd->command_name() == "delete-file")
            {
                std::error_code ec;
                fs::remove(p, ec);
                if (ec)
                    throw std::runtime_error("delete-file failed");
                return;
            }
        }

        for (Command *cmd : commands)
        {
            if (cmd->command_name() == "create-file" ||
                cmd->command_name() == "delete-file")
                continue;

            if (is_text_command(cmd->command_name()) ||
                is_symbol_command(cmd->command_name()))
                cmd->execute(orig);
            else
                throw std::runtime_error("unexpected command: " +
                                         cmd->command_name());
        }

        if (!commands.empty())
            write_file_lines(p, orig);
    }
} // namespace

void apply_sections(const std::vector<std::unique_ptr<Command>> &commands)
{
    std::vector<std::string> files;
    files.reserve(commands.size());
    for (auto &cmd : commands)
        files.push_back(cmd->filepath());

    std::sort(files.begin(), files.end());
    files.erase(std::unique(files.begin(), files.end()), files.end());

    std::map<std::string, Backup> backup;

    for (auto &f : files)
    {
        Backup b;
        fs::path p = f;

        std::error_code ec;
        if (fs::exists(p, ec))
        {
            b.existed = true;

            try
            {
                b.data = read_file_bytes(p);
            }
            catch (...)
            {
                throw std::runtime_error("cannot read original file: " + f);
            }

            auto status = fs::status(p, ec);
            if (!ec)
            {
                b.permissions = status.permissions();
                b.has_permissions = true;
            }

            auto mtime = fs::last_write_time(p, ec);
            if (!ec)
            {
                b.last_write_time = mtime;
                b.has_last_write_time = true;
            }
        }
        else
        {
            b.existed = false;
        }

        backup[f] = std::move(b);
    }

    Command *current_command = nullptr;
    std::vector<std::string> rollback_errors;

    try
    {
        for (const auto &cmd : commands)
        {
            current_command = cmd.get();
            std::vector<Command *> single{cmd.get()};
            apply_for_file(cmd->filepath(), single);
        }
    }
    catch (const std::exception &e)
    {
        for (auto &[path, b] : backup)
        {
            std::string err = restore_backup(path, b);
            if (!err.empty())
                rollback_errors.push_back(std::move(err));
        }

        if (!rollback_errors.empty() || current_command)
        {
            std::ostringstream oss;
            oss << e.what();

            if (current_command)
            {
                const Section &s = current_command->data();
                if (!s.comment.empty())
                    oss << "\nsection comment: " << s.comment;
                if (!s.marker.empty())
                {
                    oss << "\nsection marker preview:\n";
                    size_t max_preview_lines = 3;
                    for (size_t i = 0;
                         i < s.marker.size() && i < max_preview_lines;
                         ++i)
                    {
                        oss << s.marker[i] << "\n";
                    }
                }
            }

            if (!rollback_errors.empty())
            {
                oss << "\nrollback errors:\n";
                for (const auto &msg : rollback_errors)
                    oss << "  " << msg << "\n";
            }

            throw std::runtime_error(oss.str());
        }

        throw;
    }
    catch (...)
    {
        for (auto &[path, b] : backup)
        {
            std::string err = restore_backup(path, b);
            if (!err.empty())
                rollback_errors.push_back(std::move(err));
        }

        std::ostringstream oss;
        oss << "unknown error while applying patch";
        if (current_command)
            oss << " in file '" << current_command->filepath() << "'";

        if (!rollback_errors.empty())
        {
            oss << "\nrollback errors:\n";
            for (const auto &msg : rollback_errors)
                oss << "  " << msg << "\n";
        }

        throw std::runtime_error(oss.str());
    }
}
