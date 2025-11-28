#pragma once

#include "command.h"
#include "section.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace text_commands_detail
{
    class SimpleInsertCommand : public Command
    {
    public:
        explicit SimpleInsertCommand(const Section &s, bool prepend);
        void execute(std::vector<std::string> &lines) override;

    private:
        const Section &section;
        bool prepend_mode = false;
    };

    class MarkerTextCommand : public Command
    {
    public:
        MarkerTextCommand(const Section &s, std::string path);
        void execute(std::vector<std::string> &lines) override;

    protected:
        virtual bool should_indent_payload() const
        {
            return false;
        }

    private:
        std::vector<std::string>
        prepare_payload(const std::vector<std::string> &lines,
                        std::size_t begin) const;
        virtual void apply(std::vector<std::string> &lines,
                           std::size_t begin,
                           std::size_t end,
                           const std::vector<std::string> &payload) = 0;

    protected:
        const Section &section;
        std::string filepath;
    };

    class InsertAfterTextCommand : public MarkerTextCommand
    {
    public:
        InsertAfterTextCommand(const Section &s, std::string path);

    protected:
        bool should_indent_payload() const override
        {
            return true;
        }
        void apply(std::vector<std::string> &lines,
                   std::size_t begin,
                   std::size_t end,
                   const std::vector<std::string> &payload) override;
    };

    class InsertBeforeTextCommand : public MarkerTextCommand
    {
    public:
        InsertBeforeTextCommand(const Section &s, std::string path);

    protected:
        bool should_indent_payload() const override
        {
            return true;
        }
        void apply(std::vector<std::string> &lines,
                   std::size_t begin,
                   std::size_t end,
                   const std::vector<std::string> &payload) override;
    };

    class ReplaceTextCommand : public MarkerTextCommand
    {
    public:
        ReplaceTextCommand(const Section &s, std::string path);

    protected:
        bool should_indent_payload() const override
        {
            return true;
        }
        void apply(std::vector<std::string> &lines,
                   std::size_t begin,
                   std::size_t end,
                   const std::vector<std::string> &payload) override;
    };

    class DeleteTextCommand : public MarkerTextCommand
    {
    public:
        DeleteTextCommand(const Section &s, std::string path);

    private:
        void apply(std::vector<std::string> &lines,
                   std::size_t begin,
                   std::size_t end,
                   const std::vector<std::string> &payload) override;
    };

    using CommandFactory = std::function<std::unique_ptr<Command>(
        const Section &, const std::string &)>;
} // namespace text_commands_detail
