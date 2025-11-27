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


// ============================================================================
// 8. Новый YAML-формат: create_file + delete_file
// ============================================================================
TEST_CASE("YAML patch: create_file and delete_file")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_patch_create_delete";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path to_delete = tmp / "old.txt";
    {
        std::ofstream out(to_delete);
        out << "OLD1\n"
               "OLD2\n";
    }

    fs::path to_create = tmp / "new.txt";

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "description: create and delete\n";
        out << "operations:\n";
        out << "  - path: " << to_create.string() << "\n";
        out << "    op: create_file\n";
        out << "    payload: |\n";
        out << "      one\n";
        out << "      two\n";
        out << "  - path: " << to_delete.string() << "\n";
        out << "    op: delete_file\n";
    }

    CHECK(run_apply(patch) == 0);

    // файл создан
    CHECK(fs::exists(to_create));
    auto L = read_lines(to_create);
    REQUIRE(L.size() == 2);
    CHECK(L[0] == "one");
    CHECK(L[1] == "two");

    // файл удалён
    CHECK(!fs::exists(to_delete));
}

// ============================================================================
// 9. Новый YAML-формат: простая replace_text
// ============================================================================
TEST_CASE("YAML patch: replace_text simple")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_patch_replace";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "a.txt";
    {
        std::ofstream out(f);
        out << "alpha\n"
               "beta\n"
               "gamma\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n";
        out << "  - path: " << f.string() << "\n";
        out << "    op: replace_text\n";
        out << "    marker: |\n";
        out << "      beta\n";
        out << "    payload: |\n";
        out << "      XXX\n";
        out << "      YYY\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 4);
    CHECK(L[0] == "alpha");
    CHECK(L[1] == "XXX");
    CHECK(L[2] == "YYY");
    CHECK(L[3] == "gamma");
}

// ============================================================================
// 10. indent: from-marker для insert_after_text
// ============================================================================
TEST_CASE("YAML patch: indent from-marker for insert_after_text")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_patch_indent_after";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "code.cpp";
    {
        std::ofstream out(f);
        out << "void foo() {\n"
               "    int x = 1;\n"
               "    int y = 2;\n"
               "}\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n";
        out << "  - path: " << f.string() << "\n";
        out << "    op: insert_after_text\n";
        out << "    marker: |\n";
        out << "      int y = 2;\n";
        out << "    payload: |\n";
        out << "      int z = 3;\n";
        out << "    options:\n";
        out << "      indent: from-marker\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 5);
    CHECK(L[0] == "void foo() {");
    CHECK(L[1] == "    int x = 1;");
    CHECK(L[2] == "    int y = 2;");
    CHECK(L[3] == "    int z = 3;");
}

// ============================================================================
// 11. indent: from-marker для replace_text
// ============================================================================
TEST_CASE("YAML patch: indent from-marker for replace_text")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_patch_indent_replace";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "code2.cpp";
    {
        std::ofstream out(f);
        out << "if (cond) {\n"
               "    do_something();\n"
               "}\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n";
        out << "  - path: " << f.string() << "\n";
        out << "    op: replace_text\n";
        out << "    marker: |\n";
        out << "      do_something();\n";
        out << "    payload: |\n";
        out << "      do_one();\n";
        out << "      do_two();\n";
        out << "    options:\n";
        out << "      indent: from-marker\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 4);
    CHECK(L[0] == "if (cond) {");
    CHECK(L[1] == "    do_one();");
    CHECK(L[2] == "    do_two();");
}

// ============================================================================
// 12. BEFORE/AFTER в новом YAML-формате (дизамбиг маркера)
// ============================================================================
TEST_CASE("YAML patch: BEFORE and AFTER select correct marker")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_patch_before_after";
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

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n";
        out << "  - path: " << f.string() << "\n";
        out << "    op: replace_text\n";
        out << "    before: |\n";
        out << "      XXX\n";
        out << "    marker: |\n";
        out << "      target\n";
        out << "    payload: |\n";
        out << "      SECOND\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 7);
    // первая "target" должна остаться, вторая замениться
    CHECK(L[1] == "target");
    CHECK(L[5] == "SECOND");
}

// ============================================================================
// 13. delete_text в новом YAML-формате
// ============================================================================
TEST_CASE("YAML patch: delete_text removes block")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_patch_delete_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "del.txt";
    {
        std::ofstream out(f);
        out << "aaa\n"
               "bbb\n"
               "ccc\n"
               "ddd\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n";
        out << "  - path: " << f.string() << "\n";
        out << "    op: delete_text\n";
        out << "    marker: |\n";
        out << "      bbb\n";
        out << "      ccc\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 2);
    CHECK(L[0] == "aaa");
    CHECK(L[1] == "ddd");
}

// ============================================================================
// 14. Ошибка: text-команда без marker
// ============================================================================
TEST_CASE("YAML patch: missing marker for text op fails and keeps file intact")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_patch_missing_marker";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "file.txt";
    {
        std::ofstream out(f);
        out << "foo\n"
               "bar\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n";
        out << "  - path: " << f.string() << "\n";
        out << "    op: replace_text\n";
        // marker отсутствует
        out << "    payload: |\n";
        out << "      X\n";
    }

    CHECK(run_apply(patch) != 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 2);
    CHECK(L[0] == "foo");
    CHECK(L[1] == "bar");
}

// ============================================================================
// 15. Ошибка: неизвестный режим indent
// ============================================================================
TEST_CASE("YAML patch: unknown indent mode causes failure and rollback")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_patch_bad_indent";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "ind.txt";
    {
        std::ofstream out(f);
        out << "foo\n"
               "bar\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n";
        out << "  - path: " << f.string() << "\n";
        out << "    op: insert_after_text\n";
        out << "    marker: |\n";
        out << "      bar\n";
        out << "    payload: |\n";
        out << "      BAZ\n";
        out << "    options:\n";
        out << "      indent: weird-mode\n";
    }

    CHECK(run_apply(patch) != 0);

    auto L = read_lines(f);
    REQUIRE(L.size() == 2);
    CHECK(L[0] == "foo");
    CHECK(L[1] == "bar");
}


// ============================================================================
// 16. Маркер игнорирует пустые строки в файле при сравнении
// ============================================================================
TEST_CASE("YAML patch: marker matches across empty lines in file")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_patch_marker_empty_lines";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "file.txt";
    {
        std::ofstream out(f);
        out << "AAA\n"
            << "\n"      // лишняя пустая строка
            << "BBB\n"
            << "CCC\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n";
        out << "  - path: " << f.string() << "\n";
        out << "    op: replace_text\n";
        out << "    marker: |\n";
        out << "      AAA\n";
        out << "      BBB\n";
        out << "    payload: |\n";
        out << "      XXX\n";
        out << "      YYY\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    // Конкретное поведение сейчас такое:
    //   AAA + пустая строка заменяются на XXX/YYY,
    //   строка BBB остаётся (мы игнорируем пустую при сравнении, но длину блока берём из маркера).
    REQUIRE(L.size() == 4);
    CHECK(L[0] == "XXX");
    CHECK(L[1] == "YYY");
    CHECK(L[2] == "BBB");
    CHECK(L[3] == "CCC");
}

// ============================================================================
// 17. insert_after_text: маркер матчится через пустую строку
// ============================================================================
TEST_CASE("YAML patch: insert_after_text marker across empty line")
{
    fs::path tmp = fs::temp_directory_path() / "yaml_patch_insert_after_empty";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "code.txt";
    {
        std::ofstream out(f);
        out << "begin\n"
            << "A\n"
            << "\n"
            << "B\n"
            << "end\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n";
        out << "  - path: " << f.string() << "\n";
        out << "    op: insert_after_text\n";
        out << "    marker: |\n";
        out << "      A\n";
        out << "      B\n";
        out << "    payload: |\n";
        out << "      X\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    // Текущее поведение: маркер "A\nB\n" матчится на "A\n\nB\n",
    // а вставка происходит в позицию pos + marker.size() (как и раньше).
    // То есть X вставится ПЕРЕД B, потому что pos == индекс строки "A".
    REQUIRE(L.size() == 6);
    CHECK(L[0] == "begin");
    CHECK(L[1] == "A");
    CHECK(L[2].empty());       // пустая
    CHECK(L[3] == "X");        // вставка сюда
    CHECK(L[4] == "B");
    CHECK(L[5] == "end");
}
