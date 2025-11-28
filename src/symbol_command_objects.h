#pragma once

#include "command.h"
#include "section.h"
#include "symbols.h"
#include "yaml/trent.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace symbol_commands_detail
{
    class RegionReplaceCommand : public Command
    {
    public:
        explicit RegionReplaceCommand(const std::string &name);
        void parse(const nos::trent &tr) override;
        void execute(std::vector<std::string> &lines) override;

    protected:
        virtual bool find_region(const std::string &text, Region &r) const = 0;
        virtual std::string not_found_error() const = 0;
        virtual std::string invalid_region_error() const = 0;

    };

    class ReplaceCppClassCommand : public RegionReplaceCommand
    {
    public:
        ReplaceCppClassCommand();
        void parse(const nos::trent &tr) override;

    protected:
        bool find_region(const std::string &text, Region &r) const override;
        std::string not_found_error() const override;
        std::string invalid_region_error() const override;
    };

    class ReplaceCppMethodCommand : public RegionReplaceCommand
    {
    public:
        ReplaceCppMethodCommand();
        void parse(const nos::trent &tr) override;

    protected:
        bool find_region(const std::string &text, Region &r) const override;
        std::string not_found_error() const override;
        std::string invalid_region_error() const override;

    private:
        std::string cls;
        std::string method;
    };

    class ReplacePyClassCommand : public RegionReplaceCommand
    {
    public:
        ReplacePyClassCommand();
        void parse(const nos::trent &tr) override;

    protected:
        bool find_region(const std::string &text, Region &r) const override;
        std::string not_found_error() const override;
        std::string invalid_region_error() const override;
    };

    class ReplacePyMethodCommand : public RegionReplaceCommand
    {
    public:
        ReplacePyMethodCommand();
        void parse(const nos::trent &tr) override;

    protected:
        bool find_region(const std::string &text, Region &r) const override;
        std::string not_found_error() const override;
        std::string invalid_region_error() const override;

    private:
        std::string cls;
        std::string method;
    };

    using CommandFactory =
        std::function<std::unique_ptr<Command>()>;
} // namespace symbol_commands_detail
