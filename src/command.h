#pragma once

#include "parser.h"
#include "yaml/trent.h"

#include <iosfwd>
#include <string>
#include <vector>

struct ApplyOptions;
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

    std::size_t chars_inserted() const { return chars_inserted_; }
    std::size_t chars_removed() const { return chars_removed_; }

    void set_patch_language(const std::string &lang) { language_ = lang; }

    // Optional hook for additional debug info (e.g. marker preview).
    virtual void append_debug_info(std::ostream &os) const;

    friend void apply_sections(const std::vector<std::unique_ptr<Command>> &commands,
                               const ApplyOptions &options);

protected:
    static std::size_t count_total_chars(const std::vector<std::string> &lines);
    void record_effect(std::size_t before_size, std::size_t after_size);

    std::string filepath_;
    std::string name_;
    std::string language_;
    std::string comment_;
    Status status_ = Status::NotRun;
    std::string error_message_;
    std::size_t chars_inserted_ = 0;
    std::size_t chars_removed_ = 0;
};
