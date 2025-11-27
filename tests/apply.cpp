#include "doctest/doctest.h"
#include "apply_chunk_v2.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::vector<std::string> read_lines(const fs::path &p)
{
    std::ifstream in(p);
    std::vector<std::string> v;
    std::string s;
    while (std::getline(in, s))
        v.push_back(s);
    return v;
}

int run_apply(const fs::path &patch)
{
    std::string arg0 = "apply";
    std::string arg1 = patch.string();

    // храним строки в живом виде
    std::vector<std::string> args = {arg0, arg1};

    // формируем argv как указатели НА ЖИВЫЕ строки
    std::vector<char *> argv_real;
    argv_real.reserve(args.size());
    for (auto &s : args)
        argv_real.push_back(s.data());

    return apply_chunk_main((int)argv_real.size(), argv_real.data());
}

TEST_CASE("apply_chunk_main: insert-after-text")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_insert_after_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "a.txt";
    {
        std::ofstream out(f);
        out << "LINE1\n"
               "LINE2\n"
               "LINE3\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string()
            << " ===\n"
               "--- insert-after-text\n"
               "LINE2\n"
               "---\n"
               "AFTER\n"
               "TEXT\n"
               "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 5);
    CHECK(lines[0] == "LINE1");
    CHECK(lines[1] == "LINE2");
    CHECK(lines[2] == "AFTER");
    CHECK(lines[3] == "TEXT");
    CHECK(lines[4] == "LINE3");
}

TEST_CASE("apply_chunk_main: insert-before-text")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_insert_before_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "b.txt";
    {
        std::ofstream out(f);
        out << "AAA\n"
               "BBB\n"
               "CCC\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string()
            << " ===\n"
               "--- insert-before-text\n"
               "BBB\n"
               "---\n"
               "X\n"
               "Y\n"
               "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 5);
    CHECK(lines[0] == "AAA");
    CHECK(lines[1] == "X");
    CHECK(lines[2] == "Y");
    CHECK(lines[3] == "BBB");
    CHECK(lines[4] == "CCC");
}

TEST_CASE("apply_chunk_main: replace-text")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_replace_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "c.txt";
    {
        std::ofstream out(f);
        out << "alpha\n"
               "beta\n"
               "gamma\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string()
            << " ===\n"
               "--- replace-text\n"
               "beta\n"
               "---\n"
               "BETA1\n"
               "BETA2\n"
               "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0] == "alpha");
    CHECK(lines[1] == "BETA1");
    CHECK(lines[2] == "BETA2");
    CHECK(lines[3] == "gamma");
}

TEST_CASE("apply_chunk_main: delete-text")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_delete_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "d.txt";
    {
        std::ofstream out(f);
        out << "one\n"
               "two\n"
               "three\n"
               "four\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string()
            << " ===\n"
               "--- delete-text\n"
               "two\n"
               "three\n"
               "---\n"
               "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "one");
    CHECK(lines[1] == "four");
}

TEST_CASE("apply_chunk_main: delete-file then create-file")
{
    namespace fs = std::filesystem;

    fs::path tmp = fs::temp_directory_path() / "chunk_test_del_create";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "x.txt";

    // Исходный файл
    {
        std::ofstream out(f);
        out << "OLD";
    }

    // Патч: удалить → создать заново
    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "=== file: " << f.string()
            << " ===\n"
               "--- delete-file\n"
               "=END=\n"
               "\n"
               "=== file: "
            << f.string()
            << " ===\n"
               "--- create-file\n"
               "NEW1\n"
               "NEW2\n"
               "=END=\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    REQUIRE(fs::exists(f));

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "NEW1");
    CHECK(lines[1] == "NEW2");
}
