#pragma once

#include <string>

struct ClipboardReader
{
    const char *name;    // человекочитаемое имя
    const char *command; // команда для popen/_popen
    bool (*is_available)(); // опциональная проверка среды, nullptr если не
                            // нужна
};

bool run_clipboard_reader(const ClipboardReader &reader,
                          std::string &out,
                          bool verbose);

bool read_clipboard(std::string &out, bool verbose);