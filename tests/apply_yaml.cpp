#include "apply_chunk_v2.h"
#include "doctest/doctest.h"
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
    std::string a1 = patch.string();

    std::vector<std::string> store = {a0, a1};
    std::vector<char *> argv;
    for (auto &s : store)
        argv.push_back(s.data());

    return apply_chunk_main((int)argv.size(), argv.data());
}

// ============================================================================
// 1. MARKER: без BEFORE/AFTER — работает как старый режим
// ============================================================================
TEST_CASE("YAML: only MARKER: behaves like legacy replace-text")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_marker_only_test";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "a.txt";
    {
        std::ofstream out(f);
        out << "A\nB\nC\n";
    }

    fs::path patch = tmp / "patch1.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string()
            << " ===\n"
               "--- replace-text\n"
               "MARKER:\n"
               "B\n"
               "---\n"
               "X\n"
               "=END=\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 3);
    CHECK(L[0] == "A");
    CHECK(L[1] == "X");
    CHECK(L[2] == "C");
}

// ============================================================================
// 2. BEFORE fuzzy
// ============================================================================
TEST_CASE("YAML: BEFORE fuzzy selects the correct marker")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_before_test2";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "b.txt";
    {
        std::ofstream out(f);
        out << "foo\n"
               "target\n"
               "bar\n"
               "\n"
               "XXX\n"
               "target\n"
               "YYY\n";
    }

    fs::path patch = tmp / "patch2.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string()
            << " ===\n"
               "--- replace-text\n"
               "BEFORE:\n"
               "XXX\n"
               "MARKER:\n"
               "target\n"
               "---\n"
               "SECOND\n"
               "=END=\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 7);
    CHECK(L[5] == "SECOND");
}

// ============================================================================
// 3. AFTER fuzzy
// ============================================================================
TEST_CASE("YAML: AFTER fuzzy selects correct block")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_after_test2";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "c.txt";
    {
        std::ofstream out(f);
        out << "log\n"
               "X\n"
               "done\n"
               "\n"
               "log\n"
               "X\n"
               "finish\n";
    }

    fs::path patch = tmp / "patch3.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string()
            << " ===\n"
               "--- replace-text\n"
               "MARKER:\n"
               "X\n"
               "AFTER:\n"
               "finish\n"
               "---\n"
               "CHANGED\n"
               "=END=\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 7);
    CHECK(L[5] == "CHANGED");
}

// ============================================================================
// 4. BEFORE + AFTER together
// ============================================================================
TEST_CASE("YAML: strong fuzzy match with BEFORE + AFTER")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_before_after_test2";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "d.txt";
    {
        std::ofstream out(f);
        out << "A\n"
               "mark\n"
               "B\n"
               "\n"
               "C\n"
               "mark\n"
               "D\n";
    }

    fs::path patch = tmp / "patch4.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string()
            << " ===\n"
               "--- replace-text\n"
               "BEFORE:\n"
               "C\n"
               "MARKER:\n"
               "mark\n"
               "AFTER:\n"
               "D\n"
               "---\n"
               "SELECTED\n"
               "=END=\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 7);
    CHECK(L[5] == "SELECTED");
}

// ============================================================================
// 7. Legacy format still works
// ============================================================================
TEST_CASE("YAML: legacy replace-text still works")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_legacy_test2";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "g.txt";
    {
        std::ofstream out(f);
        out << "1\n"
               "2\n"
               "3\n";
    }

    fs::path patch = tmp / "patch7.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string()
            << " ===\n"
               "--- replace-text\n"
               "2\n"
               "---\n"
               "X\n"
               "=END=\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    CHECK(L[1] == "X");
}
