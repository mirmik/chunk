#include "guard/guard.h"
#include "apply.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::vector<std::string> read_lines(const fs::path &p)
{
    std::ifstream in(p);
    std::vector<std::string> v;
    std::string s;
    while (std::getline(in, s))
        v.push_back(s);
    return v;
}

static int run_apply(const fs::path &patch)
{
    std::string a0 = "apply";
    std::string a1 = "--quiet";
    std::string a2 = patch.string();

    std::vector<std::string> store = {a0, a1, a2};
    std::vector<char *> argv;
    argv.reserve(store.size());
    for (auto &s : store)
        argv.push_back(s.data());

    return apply_chunk_main((int)argv.size(), argv.data());
}

static std::string yaml_path(const fs::path &p)
{
    return p.generic_string();
}

TEST_CASE("apply_chunk_main: replace_xml_block replaces single xml element")
{
    fs::path tmp =
        fs::temp_directory_path() / "chunk_test_replace_xml_block_simple";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "config.xml";
    {
        std::ofstream out(f);
        out << "<root>\n"
               "  <item id=\"a\">\n"
               "    OLD\n"
               "  </item>\n"
               "  <item id=\"b\">\n"
               "    KEEP\n"
               "  </item>\n"
               "</root>\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: replace_xml_block\n"
               "    path: \"" << yaml_path(f) << "\"\n"
               "    marker: |\n"
               "      <item id=\"a\">\n"
               "    payload: |\n"
               "      <item id=\"a\">\n"
               "        NEW1\n"
               "        NEW2\n"
               "      </item>\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    bool has_new1 = false;
    bool has_new2 = false;
    bool has_old = false;
    bool has_keep = false;

    for (const auto &line : lines)
    {
        if (line.find("NEW1") != std::string::npos)
            has_new1 = true;
        if (line.find("NEW2") != std::string::npos)
            has_new2 = true;
        if (line.find("OLD") != std::string::npos)
            has_old = true;
        if (line.find("KEEP") != std::string::npos)
            has_keep = true;
    }

    CHECK(has_new1);
    CHECK(has_new2);
    CHECK_FALSE(has_old);
    CHECK(has_keep);
}

TEST_CASE(
    "apply_chunk_main: replace_xml_block handles nested elements with same tag")
{
    fs::path tmp =
        fs::temp_directory_path() / "chunk_test_replace_xml_block_nested";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "layout.xml";
    {
        std::ofstream out(f);
        out << "<root>\n"
               "  <item id=\"outer\">\n"
               "    <item id=\"inner\">\n"
               "      INNER\n"
               "    </item>\n"
               "    TAIL\n"
               "  </item>\n"
               "  <item id=\"other\">\n"
               "    KEEP\n"
               "  </item>\n"
               "</root>\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: replace_xml_block\n"
               "    path: \"" << yaml_path(f) << "\"\n"
               "    marker: |\n"
               "      <item id=\"outer\">\n"
               "    payload: |\n"
               "      <item id=\"outer\">\n"
               "        NEW-OUTER\n"
               "      </item>\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    bool has_inner = false;
    bool has_tail = false;
    bool has_new_outer = false;
    bool has_other_keep = false;

    for (const auto &line : lines)
    {
        if (line.find("INNER") != std::string::npos)
            has_inner = true;
        if (line.find("TAIL") != std::string::npos)
            has_tail = true;
        if (line.find("NEW-OUTER") != std::string::npos)
            has_new_outer = true;
        if (line.find("KEEP") != std::string::npos)
            has_other_keep = true;
    }

    CHECK(has_new_outer);
    CHECK_FALSE(has_inner);
    CHECK_FALSE(has_tail);
    CHECK(has_other_keep);
}
