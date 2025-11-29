#include "runner.h"

#include "command.h"
#include "file_io.h"
#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace
{
    struct FileState
    {
        std::string path;

        bool existed_before = false;   // было на диске до патча
        bool exists_now = false;       // логическое «существует» после применения команд в памяти

        std::string original_bytes;     // для бэкапа на случай проблем при коммите
        std::vector<std::string> lines; // текущее текстовое представление файла (если exists_now == true)

        fs::file_time_type last_write_time;
        fs::perms permissions = fs::perms::unknown;
        bool has_last_write_time = false;
        bool has_permissions = false;
    };

    std::string restore_backup(const FileState &state)
    {
        fs::path p = state.path;
        std::error_code ec;

        if (!state.existed_before)
        {
            fs::remove(p, ec);
            if (ec)
            {
                return "rollback: failed to remove file '" + state.path +
                       "': " + ec.message();
            }
            return {};
        }

        try
        {
            write_file_bytes(p, state.original_bytes);
        }
        catch (const std::exception &e)
        {
            return "rollback: failed to restore data for file '" +
                   state.path + "': " + e.what();
        }
        catch (...)
        {
            return "rollback: failed to restore data for file '" +
                   state.path + "': unknown error";
        }

        if (state.has_permissions)
        {
            std::error_code perm_ec;
            fs::permissions(
                p, state.permissions, fs::perm_options::replace, perm_ec);
            if (perm_ec)
            {
                return "rollback: failed to restore permissions for file '" +
                       state.path + "': " + perm_ec.message();
            }
        }

        if (state.has_last_write_time)
        {
            std::error_code time_ec;
            fs::last_write_time(p, state.last_write_time, time_ec);
            if (time_ec)
            {
                return "rollback: failed to restore timestamp for file '" +
                       state.path + "': " + time_ec.message();
            }
        }

        return {};
    }
} // namespace

void apply_sections(const std::vector<std::unique_ptr<Command>> &commands,
                    const ApplyOptions &options)
{
    // Собираем список файлов, которые затрагивает патч.
    std::map<std::string, FileState> files;

    for (const auto &cmd : commands)
    {
        const std::string &path = cmd->filepath();
        if (path.empty())
            continue;

        auto it = files.find(path);
        if (it == files.end())
        {
            FileState st;
            st.path = path;
            files.emplace(path, std::move(st));
        }
    }

    // Снимок состояния файлов до применения патча.
    for (auto &[path, state] : files)
    {
        fs::path p = path;
        std::error_code ec;
        if (fs::exists(p, ec))
        {
            state.existed_before = true;
            state.exists_now = true;

            try
            {
                state.original_bytes = read_file_bytes(p);
                state.lines = read_file_lines(p);
            }
            catch (...)
            {
                throw std::runtime_error("cannot read original file: " + path);
            }

            auto status = fs::status(p, ec);
            if (!ec)
            {
                state.permissions = status.permissions();
                state.has_permissions = true;
            }

            auto mtime = fs::last_write_time(p, ec);
            if (!ec)
            {
                state.last_write_time = mtime;
                state.has_last_write_time = true;
            }
        }
        else
        {
            state.existed_before = false;
            state.exists_now = false;
            state.original_bytes.clear();
            state.lines.clear();
        }
    }

    // Этап 1: применяем все команды в памяти, не трогая диск.
    Command *current_command = nullptr;

    try
    {
        for (const auto &cmd_ptr : commands)
        {
            current_command = cmd_ptr.get();
            const std::string &path = current_command->filepath();
            if (path.empty())
                continue;

            auto it = files.find(path);
            if (it == files.end())
                throw std::runtime_error("internal error: no file state");
            FileState &state = it->second;
            const std::string &name = current_command->command_name();
            if (name == "delete-file")
            {
                current_command->reset_status();
                if (!state.exists_now)
                {
                    current_command->mark_failed("delete-file: file does not exist");
                    if (!options.ignore_failures)
                        throw std::runtime_error(current_command->error_message());
                    continue;
                }
                std::vector<std::string> before = state.lines;
                state.exists_now = false;
                state.lines.clear();
                current_command->record_effect(before, state.lines);
                current_command->mark_success();
                continue;
            }
            if (!state.exists_now)
                state.lines.clear();
            current_command->run(state.lines);
            if (current_command->status() == Command::Status::Failed)
            {
                if (!options.ignore_failures)
                    throw std::runtime_error(current_command->error_message());
            }
            else
            {
                state.exists_now = true;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::ostringstream oss;
        oss << e.what();

        if (current_command)
        {
            if (!current_command->comment().empty())
                oss << "\nsection comment: " << current_command->comment();
            current_command->append_debug_info(oss);
        }

        throw std::runtime_error(oss.str());
    }
    catch (...)
    {
        std::ostringstream oss;
        oss << "unknown error while applying patch";
        if (current_command)
            oss << " in file '" << current_command->filepath() << "'";

        throw std::runtime_error(oss.str());
    }

    if (options.dry_run)
        return;
    // Этап 2: коммитим изменения на диск. Если что-то пошло не так — откатываем.
    std::vector<std::string> rollback_errors;

    try
    {
        for (auto &[path, state] : files)
        {
            fs::path p = path;
            std::error_code ec;

            if (!state.exists_now)
            {
                if (state.existed_before)
                {
                    fs::remove(p, ec);
                    if (ec)
                        throw std::runtime_error("delete-file failed");
                }
                continue;
            }

            write_file_lines(p, state.lines);

            if (state.has_permissions)
            {
                std::error_code perm_ec;
                fs::permissions(
                    p, state.permissions, fs::perm_options::replace, perm_ec);
                if (perm_ec)
                {
                    throw std::runtime_error(
                        "failed to restore permissions for file '" + path +
                        "': " + perm_ec.message());
                }
            }

            if (state.has_last_write_time)
            {
                std::error_code time_ec;
                fs::last_write_time(p, state.last_write_time, time_ec);
                if (time_ec)
                {
                    throw std::runtime_error(
                        "failed to restore timestamp for file '" + path +
                        "': " + time_ec.message());
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        for (auto &[path, state] : files)
        {
            std::string err = restore_backup(state);
            if (!err.empty())
                rollback_errors.push_back(std::move(err));
        }

        std::ostringstream oss;
        oss << e.what();

        if (!rollback_errors.empty())
        {
            oss << "\nrollback errors:\n";
            for (const auto &msg : rollback_errors)
                oss << "  " << msg << "\n";
        }

        throw std::runtime_error(oss.str());
    }
    catch (...)
    {
        for (auto &[path, state] : files)
        {
            std::string err = restore_backup(state);
            if (!err.empty())
                rollback_errors.push_back(std::move(err));
        }

        std::ostringstream oss;
        oss << "unknown error while writing files";

        if (!rollback_errors.empty())
        {
            oss << "\nrollback errors:\n";
            for (const auto &msg : rollback_errors)
                oss << "  " << msg << "\n";
        }

        throw std::runtime_error(oss.str());
    }
}
