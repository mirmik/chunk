#pragma once

#include "section.h"
#include "symbols.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace symbol_commands_detail
{
class SymbolCommand
{
public:
    virtual ~SymbolCommand() = default;
    virtual void execute(std::vector<std::string> &lines) = 0;
};

class RegionReplaceCommand : public SymbolCommand
{
public:
    RegionReplaceCommand(const Section &s, std::string path);
    void execute(std::vector<std::string> &lines) override;

protected:
    virtual bool find_region(const std::string &text, Region &r) const = 0;
    virtual std::string not_found_error() const = 0;
    virtual std::string invalid_region_error() const = 0;

    const Section &section;
    std::string filepath;
};

class ReplaceCppClassCommand : public RegionReplaceCommand
{
public:
    ReplaceCppClassCommand(const Section &s, std::string path);

protected:
    bool find_region(const std::string &text, Region &r) const override;
    std::string not_found_error() const override;
    std::string invalid_region_error() const override;
};

class ReplaceCppMethodCommand : public RegionReplaceCommand
{
public:
    ReplaceCppMethodCommand(const Section &s, std::string path);

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
    ReplacePyClassCommand(const Section &s, std::string path);

protected:
    bool find_region(const std::string &text, Region &r) const override;
    std::string not_found_error() const override;
    std::string invalid_region_error() const override;
};

class ReplacePyMethodCommand : public RegionReplaceCommand
{
public:
    ReplacePyMethodCommand(const Section &s, std::string path);

protected:
    bool find_region(const std::string &text, Region &r) const override;
    std::string not_found_error() const override;
    std::string invalid_region_error() const override;

private:
    std::string cls;
    std::string method;
};

using SymbolCommandFactory =
    std::function<std::unique_ptr<SymbolCommand>(const Section &, const std::string &)>;
} // namespace symbol_commands_detail
