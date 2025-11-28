language: c++
description: "Добавить поле language в CHUNK и игнорировать комментарии к коду при поиске маркеров"
operations:
  - path: src/section.h
    op: replace_text
    comment: "Section теперь хранит язык патча"
    marker: |-
      struct Section
      {
      	std::string filepath;
      	std::string command;
    payload: |-
      struct Section
      {
      	std::string filepath;
      	std::string command;
      	std::string language;

  - path: src/section.cpp
    op: replace_text
    comment: "Читаем language из корня YAML и пробрасываем в Section"
    marker: |-
      std::vector<Section> parse_yaml_patch_text(const std::string &text)
      {
      	trent root = nos::yaml::parse(text);
      	const trent *ops_node = nullptr;
      	if (root.is_dict())
      	{
      		auto &dict = root.as_dict();
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
    payload: |-
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

  - path: src/section.cpp
    op: replace_text
    comment: "Записываем язык в каждую Section"
    marker: |-
      		Section s;
      		s.filepath = it_path->second.as_string();
      		if (s.filepath.empty())
      			throw std::runtime_error("YAML patch: 'path' must not be empty");
      		s.command = normalize_op_name(it_op->second.as_string());
      		s.seq = seq++;
      		s.comment = get_scalar(op_node, "comment");
    payload: |-
      		Section s;
      		s.filepath = it_path->second.as_string();
      		if (s.filepath.empty())
      			throw std::runtime_error("YAML patch: 'path' must not be empty");
      		s.command = normalize_op_name(it_op->second.as_string());
      		s.language = patch_language;
      		s.seq = seq++;
      		s.comment = get_scalar(op_node, "comment");

  - path: src/text_commands.cpp
    op: insert_after_text
    comment: "Нужен <cctype> для std::tolower"
    marker: |-
      #include "text_commands.h"
      #include <stdexcept>
      #include <string>
      #include <string_view>
    payload: |-
      #include "text_commands.h"
      #include <stdexcept>
      #include <string>
      #include <string_view>
      #include <cctype>

  - path: src/text_commands.cpp
    op: insert_after_text
    comment: "Добавляем язык патча и нормализацию строки с учётом комментариев"
    marker: |-
      std::string trim(std::string_view view)
      {
      	if (view.empty())
      		return "";
      
      	const char *left = view.data();
      	const char *right = view.data() + view.size() - 1;
      	const char *end = view.data() + view.size();
      	while (left != end &&
      	       (*left == ' ' || *left == '\n' || *left == '\r' || *left == '\t'))
      		++left;
      
      	if (left == end)
      		return "";
      
      	while (left != right && (*right == ' ' || *right == '\n' ||
      	                          *right == '\r' || *right == '\t'))
      		--right;
      
      	return std::string(left, (right - left) + 1);
      }
    payload: |-
      std::string trim(std::string_view view)
      {
      	if (view.empty())
      		return "";
      
      	const char *left = view.data();
      	const char *right = view.data() + view.size() - 1;
      	const char *end = view.data() + view.size();
      	while (left != end &&
      	       (*left == ' ' || *left == '\n' || *left == '\r' || *left == '\t'))
      		++left;
      
      	if (left == end)
      		return "";
      
      	while (left != right && (*right == ' ' || *right == '\n' ||
      	                          *right == '\r' || *right == '\t'))
      		--right;
      
      	return std::string(left, (right - left) + 1);
      }
      
      enum class PatchLanguage
      {
      	Unknown,
      	Cpp,
      	Python
      };
      
      PatchLanguage detect_language(const Section *s)
      {
      	if (!s)
      		return PatchLanguage::Unknown;
      	std::string lang = s->language;
      	if (lang.empty())
      		return PatchLanguage::Unknown;
      
      	for (char &ch : lang)
      		ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
      
      	if (lang == "c++" || lang == "cpp" || lang == "cxx")
      		return PatchLanguage::Cpp;
      	if (lang == "python" || lang == "py")
      		return PatchLanguage::Python;
      	return PatchLanguage::Unknown;
      }
      
      std::string strip_code_comment(std::string_view view, PatchLanguage lang)
      {
      	if (view.empty())
      		return std::string();
      
      	if (lang == PatchLanguage::Python)
      	{
      		const char *data = view.data();
      		std::size_t len = view.size();
      		for (std::size_t i = 0; i < len; ++i)
      		{
      			if (data[i] == '#')
      				return std::string(data, i);
      		}
      		return std::string(view);
      	}
      	if (lang == PatchLanguage::Cpp)
      	{
      		std::string s(view);
      		std::size_t pos = s.find("//");
      		if (pos != std::string::npos)
      			s.resize(pos);
      		return s;
      	}
      	return std::string(view);
      }
      
      std::string normalize_line_for_match(std::string_view view,
                                           PatchLanguage lang)
      {
      	std::string no_comment = strip_code_comment(view, lang);
      	return trim(no_comment);
      }

  - path: src/text_commands.cpp
    op: replace_text
    comment: "find_marker_matches теперь учитывает язык и выкидывает комментарии"
    marker: |-
      std::vector<MarkerMatch>
      find_marker_matches(const std::vector<std::string> &haystack,
      		    const std::vector<std::string> &needle)
      {
      	std::vector<MarkerMatch> matches;
      
      	std::vector<std::string> pat;
      	pat.reserve(needle.size());
      	for (const auto &s : needle)
      	{
      		std::string t = trim(s);
      		if (!t.empty())
      			pat.push_back(std::move(t));
      	}
      
      	if (pat.empty())
      		return matches;
      	std::vector<std::string> hs;
      	std::vector<int>         hs_idx;
      	hs.reserve(haystack.size());
      	hs_idx.reserve(haystack.size());
      
      	for (int i = 0; i < (int)haystack.size(); ++i)
      	{
      		std::string t = trim(haystack[i]);
      		if (!t.empty())
      		{
      			hs.emplace_back(std::move(t));
      			hs_idx.push_back(i);
      		}
      	}
      
      	if (hs.empty() || pat.size() > hs.size())
      		return matches;
      	const std::size_t n = hs.size();
      	const std::size_t m = pat.size();
      
      	for (std::size_t i = 0; i + m <= n; ++i)
      	{
      		bool ok = true;
      		for (std::size_t j = 0; j < m; ++j)
      		{
      			if (hs[i + j] != pat[j])
      			{
      				ok = false;
      				break;
      			}
      		}
      		if (ok)
      		{
      			int begin = hs_idx[i];
      			int end   = hs_idx[i + m - 1];
      			matches.push_back(MarkerMatch{begin, end});
      		}
      	}
      
      	return matches;
      }
    payload: |-
      std::vector<MarkerMatch>
      find_marker_matches(const std::vector<std::string> &haystack,
      		    const std::vector<std::string> &needle,
      		    const Section *section)
      {
      	std::vector<MarkerMatch> matches;
      
      	PatchLanguage lang = detect_language(section);
      
      	std::vector<std::string> pat;
      	pat.reserve(needle.size());
      	for (const auto &s : needle)
      	{
      		std::string t = normalize_line_for_match(s, lang);
      		if (!t.empty())
      			pat.push_back(std::move(t));
      	}
      
      	if (pat.empty())
      		return matches;
      	std::vector<std::string> hs;
      	std::vector<int>         hs_idx;
      	hs.reserve(haystack.size());
      	hs_idx.reserve(haystack.size());
      
      	for (int i = 0; i < (int)haystack.size(); ++i)
      	{
      		std::string t = normalize_line_for_match(haystack[i], lang);
      		if (!t.empty())
      		{
      			hs.emplace_back(std::move(t));
      			hs_idx.push_back(i);
      		}
      	}
      
      	if (hs.empty() || pat.size() > hs.size())
      		return matches;
      	const std::size_t n = hs.size();
      	const std::size_t m = pat.size();
      
      	for (std::size_t i = 0; i + m <= n; ++i)
      	{
      		bool ok = true;
      		for (std::size_t j = 0; j < m; ++j)
      		{
      			if (hs[i + j] != pat[j])
      			{
      				ok = false;
      				break;
      			}
      		}
      		if (ok)
      		{
      			int begin = hs_idx[i];
      			int end   = hs_idx[i + m - 1];
      			matches.push_back(MarkerMatch{begin, end});
      		}
      	}
      
      	return matches;
      }

  - path: src/text_commands.cpp
    op: replace_text
    comment: "find_best_marker_match сравнивает строки с учётом языка и комментариев"
    marker: |-
      int find_best_marker_match(const std::vector<std::string> &lines,
      			   const Section *s,
      			   const std::vector<MarkerMatch> &candidates)
      {
      	if (candidates.empty())
      		return -1;
      	if (s->before.empty() && s->after.empty())
      		return 0;
      
      	auto trim_eq = [&](const std::string &a, const std::string &b)
      	{ return trim(a) == trim(b); };
      
      	std::vector<int> strict;
    payload: |-
      int find_best_marker_match(const std::vector<std::string> &lines,
      			   const Section *s,
      			   const std::vector<MarkerMatch> &candidates)
      {
      	if (candidates.empty())
      		return -1;
      	if (s->before.empty() && s->after.empty())
      		return 0;
      
      	PatchLanguage lang = detect_language(s);
      	auto match_eq = [&](const std::string &a, const std::string &b)
      	{ return normalize_line_for_match(a, lang) == normalize_line_for_match(b, lang); };
      
      	std::vector<int> strict;

  - path: src/text_commands.cpp
    op: replace_text
    comment: "before-контекст: используем match_eq вместо trim_eq"
    marker: |-
      				for (int i = 0; i < need; ++i)
      				{
      					const std::string &want =
      					    s->before[static_cast<std::size_t>(need - 1 - i)];
      					const std::string &got =
      					    lines[static_cast<std::size_t>(pos - 1 - i)];
      					if (!trim_eq(got, want))
      					{
      						ok = false;
      						break;
      					}
      				}
    payload: |-
      				for (int i = 0; i < need; ++i)
      				{
      					const std::string &want =
      					    s->before[static_cast<std::size_t>(need - 1 - i)];
      					const std::string &got =
      					    lines[static_cast<std::size_t>(pos - 1 - i)];
      					if (!match_eq(got, want))
      					{
      						ok = false;
      						break;
      					}
      				}

  - path: src/text_commands.cpp
    op: replace_text
    comment: "after-контекст: используем match_eq вместо trim_eq"
    marker: |-
      				for (int i = 0; i < need; ++i)
      				{
      					const std::string &want =
      					    s->after[static_cast<std::size_t>(i)];
      					const std::string &got =
      					    lines[static_cast<std::size_t>(start + i)];
      					if (!trim_eq(got, want))
      					{
      						ok = false;
      						break;
      					}
      				}
    payload: |-
      				for (int i = 0; i < need; ++i)
      				{
      					const std::string &want =
      					    s->after[static_cast<std::size_t>(i)];
      					const std::string &got =
      					    lines[static_cast<std::size_t>(start + i)];
      					if (!match_eq(got, want))
      					{
      						ok = false;
      						break;
      					}
      				}

  - path: src/text_commands.cpp
    op: replace_text
    comment: "Передаём Section* в find_marker_matches"
    marker: |-
      	std::vector<MarkerMatch> matches = find_marker_matches(lines, s->marker);
    payload: |-
      	std::vector<MarkerMatch> matches = find_marker_matches(lines, s->marker, s);

