#pragma once

#include "section.h"
#include "yaml/trent.h"

#include <string>
#include <vector>

class Command
{
public:
    explicit Command(std::string name);
    virtual ~Command() = default;

    virtual void parse(const nos::trent &tr) = 0;
    virtual void execute(std::vector<std::string> &lines) = 0;

    const std::string &filepath() const { return section_.filepath; }
    const std::string &command_name() const { return section_.command; }
    const Section &data() const { return section_; }

    void set_patch_language(const std::string &lang)
    {
        section_.language = lang;
    }

    void set_sequence(int seq)
    {
        section_.seq = seq;
    }

    void load_section(const Section &s)
    {
        section_ = s;
    }

protected:
    Section section_;
};
