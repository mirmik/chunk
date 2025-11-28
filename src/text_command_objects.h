#pragma once

#include "command.h"
#include "section.h"
#include "yaml/trent.h"

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
        SimpleInsertCommand(const std::string &name, bool prepend);
        void parse(const nos::trent &tr) override;
        void execute(std::vector<std::string> &lines) override;

    private:
        bool prepend_mode = false;
    };

    class MarkerTextCommand : public Command
    {
    public:
        MarkerTextCommand(const std::string &name);
        void parse(const nos::trent &tr) override;
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
    };

    class InsertAfterTextCommand : public MarkerTextCommand
    {
    public:
        InsertAfterTextCommand();

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
        InsertBeforeTextCommand();

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
        ReplaceTextCommand();

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
        DeleteTextCommand();

    private:
        void apply(std::vector<std::string> &lines,
                   std::size_t begin,
                   std::size_t end,
                   const std::vector<std::string> &payload) override;
    };

    using CommandFactory =
        std::function<std::unique_ptr<Command>()>;
} // namespace text_commands_detail
