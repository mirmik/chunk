#include "section.h"

#include <stdexcept>
#include <string>
#include <yaml/yaml.h>

using nos::trent;

namespace
{
std::vector<std::string> split_scalar_lines(const std::string &text)
{
    std::vector<std::string> result;
    if (text.empty())
        return result;

    std::size_t start = 0;
    const std::size_t n = text.size();

    while (true)
    {
        std::size_t pos = text.find('\n', start);
        if (pos == std::string::npos)
        {
            if (start < n)
                result.emplace_back(text.substr(start));
            break;
        }

        result.emplace_back(text.substr(start, pos - start));
        start = pos + 1;

        if (start >= n)
            break;
    }

    return result;
}

std::string get_scalar(const trent &node, const char *key)
{
    const auto &dict = node.as_dict();
    auto it = dict.find(key);
    if (it == dict.end())
        return std::string();

    const trent &v = it->second;
    if (v.is_nil())
        return std::string();

    return v.as_string();
}

std::string normalize_op_name(const std::string &op_name)
{
    std::string name = op_name;
    for (char &ch : name)
    {
        if (ch == '_')
            ch = '-';
    }

    if (name == "create-file" ||
        name == "delete-file" ||
        name == "insert-after-text" ||
        name == "insert-before-text" ||
        name == "replace-text" ||
        name == "delete-text" ||
        name == "replace-cpp-class" ||
        name == "replace-cpp-method" ||
        name == "replace-py-class" ||
        name == "replace-py-method")
    {
        return name;
    }

    throw std::runtime_error("YAML patch: unknown op: " + op_name);
}
} // namespace

bool is_text_command(const std::string &cmd)
{
    return cmd == "insert-after-text" || cmd == "insert-before-text" ||
           cmd == "replace-text" || cmd == "delete-text";
}

bool is_symbol_command(const std::string &cmd)
{
    return cmd == "replace-cpp-method" || cmd == "replace-cpp-class" ||
           cmd == "replace-py-method" || cmd == "replace-py-class";
}

std::vector<Section> parse_yaml_patch_text(const std::string &text)
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
    std::vector<Section> sections;
    sections.reserve(ops.size());

    int seq = 0;

    for (const trent &op_node : ops)
    {
        if (!op_node.is_dict())
            throw std::runtime_error("YAML patch: each operation must be a mapping");

        const auto &m = op_node.as_dict();

        auto it_path = m.find("path");
        auto it_op   = m.find("op");
        if (it_path == m.end())
            throw std::runtime_error("YAML patch: operation missing 'path'");
        if (it_op == m.end())
            throw std::runtime_error("YAML patch: operation missing 'op'");

        Section s;
        s.filepath = it_path->second.as_string();
        if (s.filepath.empty())
        	throw std::runtime_error("YAML patch: 'path' must not be empty");
        s.command = normalize_op_name(it_op->second.as_string());
        s.language = patch_language;
        s.seq = seq++;
        s.comment = get_scalar(op_node, "comment");

        std::string marker_text  = get_scalar(op_node, "marker");
        std::string before_text  = get_scalar(op_node, "before");
        std::string after_text   = get_scalar(op_node, "after");
        std::string payload_text = get_scalar(op_node, "payload");
        std::string class_text   = get_scalar(op_node, "class");
        std::string method_text  = get_scalar(op_node, "method");
        std::string symbol_text  = get_scalar(op_node, "symbol");

        auto it_opts = m.find("options");
        if (it_opts != m.end() && !it_opts->second.is_nil())
        {
            const auto &opts = it_opts->second.as_dict();
            auto it_ind = opts.find("indent");
            if (it_ind != opts.end())
            {
                std::string mode = it_ind->second.as_string();
                if (mode == "none" || mode == "as-is")
                {
                    s.indent_from_marker = false;
                }
                else if (mode == "from-marker" || mode == "marker" || mode == "auto")
                {
                    s.indent_from_marker = true;
                }
                else
                {
                    throw std::runtime_error(
                        "YAML patch: unknown indent mode: " + mode);
                }
            }
        }

        if (s.command == "create-file")
        {
            if (!payload_text.empty())
                s.payload = split_scalar_lines(payload_text);
        }
        else if (s.command == "delete-file")
        {
        }
        else if (is_text_command(s.command))
        {
            if (marker_text.empty())
                throw std::runtime_error(
                    "YAML patch: text op '" + s.command +
                    "' for file '" + s.filepath + "' requires 'marker'");

            s.marker = split_scalar_lines(marker_text);

            if (!before_text.empty())
                s.before = split_scalar_lines(before_text);
            if (!after_text.empty())
                s.after = split_scalar_lines(after_text);
            if (s.command != "delete-text" && !payload_text.empty())
                s.payload = split_scalar_lines(payload_text);
        }
        else if (is_symbol_command(s.command))
        {
            if (payload_text.empty())
                throw std::runtime_error(
                    "YAML patch: symbol op '" + s.command +
                    "' for file '" + s.filepath + "' requires 'payload'");
            s.payload = split_scalar_lines(payload_text);

            if (s.command == "replace-cpp-class" ||
                s.command == "replace-py-class")
            {
                if (class_text.empty())
                    throw std::runtime_error(
                        "YAML patch: op '" + s.command + "' requires 'class' key");
                s.arg1 = class_text;
            }
            else if (s.command == "replace-cpp-method" ||
                     s.command == "replace-py-method")
            {
                if (!class_text.empty() && !method_text.empty())
                {
                    s.arg1 = class_text;
                    s.arg2 = method_text;
                }
                else if (!symbol_text.empty())
                {
                    s.arg1 = symbol_text;
                }
                else
                {
                    throw std::runtime_error(
                        "YAML patch: op '" + s.command +
                        "' requires 'class'+'method' or 'symbol'");
                }
            }
        }

        sections.emplace_back(std::move(s));
    }

    return sections;
}
