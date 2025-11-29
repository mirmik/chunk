#pragma once

#include "parser.h"
#include "yaml/trent.h"

#include <iosfwd>
#include <string>
#include <vector>

class Command
{
public:
    explicit Command(std::string name);
    virtual ~Command() = default;

    enum class Status
    {
        NotRun,
        Success,
        Failed
    };

    Status status() const { return status_; }
    const std::string &error_message() const { return error_message_; }

    void reset_status();
    void mark_success();
    void mark_failed(const std::string &message);
    void run(std::vector<std::string> &lines);

    virtual void parse(const nos::trent &tr) = 0;
    virtual void execute(std::vector<std::string> &lines) = 0;

    const std::string &filepath() const { return filepath_; }
    const std::string &command_name() const { return name_; }

    const std::string &comment() const { return comment_; }
    const std::string &language() const { return language_; }

    void set_patch_language(const std::string &lang) { language_ = lang; }

    // Optional hook for additional debug info (e.g. marker preview).
    virtual void append_debug_info(std::ostream &os) const;

protected:
    std::string filepath_;
    std::string name_;
    std::string language_;
    std::string comment_;
    Status status_ = Status::NotRun;
    std::string error_message_;
};
