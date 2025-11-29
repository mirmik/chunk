#include "parser.h"

#include "command.h"
#include "command_fabric.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <yaml/yaml.h>

using nos::trent;

static std::unordered_map<std::string, std::vector<std::string>> synonyms = {
    {"insert-after-text", {"insert-after-text", "insert-text-after"}},
    {"insert-before-text", {"insert-before-text", "insert-text-before"}},
    {"replace-text", {"replace-text"}},
    {"delete-text", {"delete-text"}},
    {"prepend-text", {"prepend-text"}},
    {"append-text", {"append-text"}},
    {"create-file", {"create-file"}},
    {"delete-file", {"delete-file"}},
    {"replace-cpp-class", {"replace-cpp-class"}},
    {"replace-cpp-method", {"replace-cpp-method"}},
    {"replace-cpp-method", {"replace-cpp-method"}},
    {"replace-cpp-function", {"replace-cpp-function"}},
    {"replace-py-class", {"replace-py-class"}},
    {"replace-py-method", {"replace-py-method"}},
};

static std::unordered_map<std::string, std::string> synonym_map;

namespace
{
    void ensure_synonym_map()
    {
        if (!synonym_map.empty())
            return;

        for (const auto &pair : synonyms)
        {
            const std::string &canonical = pair.first;
            for (const std::string &syn : pair.second)
            {
                synonym_map[syn] = canonical;
            }
        }
    }

    std::string normalize_op_name(const std::string &op_name)
    {
        ensure_synonym_map();

        std::string name = op_name;
        for (char &ch : name)
        {
            if (ch == '_')
                ch = '-';
        }

        auto it = synonym_map.find(name);
        if (it == synonym_map.end())
        {
            throw std::runtime_error("YAML patch: unknown operation: " +
                                     op_name);
        }

        return it->second;
    }
} // namespace

std::vector<std::unique_ptr<Command>>
parse_yaml_patch_text(const std::string &text)
{
    trent root = nos::yaml::parse(text);
    std::string patch_language;
    const trent *ops_node = nullptr;
    if (root.is_dict())
    {
        auto &dict = root.as_dict();
        auto it_lang = dict.find("language");
        if (it_lang != dict.end() && !it_lang->second.is_nil())
        {
            patch_language = it_lang->second.as_string();
        }
        auto it = dict.find("operations");
        if (it == dict.end())
            throw std::runtime_error("YAML patch: missing 'operations' key");
        ops_node = &it->second;
    }
    else if (root.is_list())
    {
        ops_node = &root;
    }
    else
    {
        throw std::runtime_error(
            "YAML patch: root must be mapping or sequence");
    }

    if (!ops_node->is_list())
        throw std::runtime_error("YAML patch: 'operations' must be a sequence");

    const auto &ops = ops_node->as_list();
    std::vector<std::unique_ptr<Command>> commands;
    commands.reserve(ops.size());

    for (const trent &op_node : ops)
    {
        if (!op_node.is_dict())
            throw std::runtime_error(
                "YAML patch: each operation must be a mapping");

        const auto &m = op_node.as_dict();

        auto it_op = m.find("op");
        if (it_op == m.end())
            throw std::runtime_error("YAML patch: operation missing 'op'");

        std::string op_name = normalize_op_name(it_op->second.as_string());
        auto cmd = make_command(op_name);
        cmd->set_patch_language(patch_language);
        cmd->parse(op_node);

        commands.emplace_back(std::move(cmd));
    }

    return commands;
}
