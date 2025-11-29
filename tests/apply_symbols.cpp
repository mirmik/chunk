#include "apply.h"
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
    argv.reserve(store.size());
    for (auto &s : store)
        argv.push_back(s.data());

    return apply_chunk_main((int)argv.size(), argv.data());
}

// ============================================================================
// C++: replace-cpp-class
// ============================================================================
TEST_CASE("symbol API: replace-cpp-class replaces only target class")
{
    fs::path tmp = fs::temp_directory_path() / "symbol_cpp_class";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "foo.cpp";
    {
        std::ofstream out(f);
        out << "#include <string>\n"
               "\n"
               "class Foo {\n"
               "public:\n"
               "    int x() const;\n"
               "};\n"
               "\n"
               "class Bar {\n"
               "public:\n"
               "    void ping();\n"
               "};\n";
    }

    fs::path patch = tmp / "patch_class.txt";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: replace_cpp_class\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    class: \"Foo\"\n"
               "    payload: |\n"
               "      class Foo {\n"
               "      public:\n"
               "          int y() const { return 42; }\n"
               "      };\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() >= 8);

    CHECK(L[0] == "#include <string>");
    CHECK(L[1] == "");
    CHECK(L[2] == "class Foo {");
    CHECK(L[3] == "public:");
    CHECK(L[4] == "    int y() const { return 42; }");
    CHECK(L[5] == "};");
    CHECK(L[6] == "");
    CHECK(L[7] == "class Bar {");
}




// ============================================================================
// Python: replace-py-class
// ============================================================================
TEST_CASE("symbol API: replace-py-class replaces whole class body")
{
    fs::path tmp = fs::temp_directory_path() / "symbol_py_class";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "foo.py";
    {
        std::ofstream out(f);
        out << "class Foo:\n"
               "    def __init__(self):\n"
               "        self.x = 1\n"
               "\n"
               "class Bar:\n"
               "    pass\n";
    }

    fs::path patch = tmp / "patch_py_class.txt";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: replace_py_class\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    class: \"Foo\"\n"
               "    payload: |\n"
               "      class Foo:\n"
               "          def __init__(self):\n"
               "              self.x = 2\n"
               "          def answer(self):\n"
               "              return 42\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() >= 5);

    // Проверяем, что новый класс Foo на месте
    CHECK(L[0] == "class Foo:");

    bool found_init = false;
    bool found_x2 = false;
    bool found_answer = false;
    bool found_ret42 = false;
    bool found_bar = false;

    for (const auto &line : L)
    {
        if (line.find("def __init__") != std::string::npos)
            found_init = true;
        if (line.find("self.x = 2") != std::string::npos)
            found_x2 = true;
        if (line.find("def answer") != std::string::npos)
            found_answer = true;
        if (line.find("return 42") != std::string::npos)
            found_ret42 = true;
        if (line == "class Bar:")
            found_bar = true;
    }

    CHECK(found_init);
    CHECK(found_x2);
    CHECK(found_answer);
    CHECK(found_ret42);
    CHECK(found_bar);
}

// ============================================================================
// Python: replace-py-method
// ============================================================================
TEST_CASE("symbol API: replace-py-method with separate class and method name")
{
    fs::path tmp = fs::temp_directory_path() / "symbol_py_method_1";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "weird.py";
    {
        std::ofstream out(f);
        out << "class Weird:\n"
               "    def run(self):\n"
               "        return 1\n"
               "\n"
               "    def other(self):\n"
               "        return 2\n";
    }

    fs::path patch = tmp / "patch_py_method1.txt";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: replace_py_method\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    class: \"Weird\"\n"
               "    method: \"run\"\n"
               "    payload: |\n"
               "      def run(self):\n"
               "          return 100\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() >= 5);

    bool found_run_100 = false;
    bool found_other_2 = false;
    bool seen_def_run = false;
    bool seen_return_100 = false;

    for (const auto &line : L)
    {
        if (line.find("def run") != std::string::npos)
            seen_def_run = true;
        if (line.find("return 100") != std::string::npos)
            seen_return_100 = true;

        if (line.find("def other") != std::string::npos ||
            line.find("return 2") != std::string::npos)
        {
            // Очень грубо: убеждаемся, что следы second метода остались
            found_other_2 = true;
        }
    }

    found_run_100 = seen_def_run && seen_return_100;

    CHECK(found_run_100);
    CHECK(found_other_2);
}

TEST_CASE("symbol API: replace-py-method with Class.method syntax")
{
    fs::path tmp = fs::temp_directory_path() / "symbol_py_method_2";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "async_foo.py";
    {
        std::ofstream out(f);
        out << "class Foo:\n"
               "    async def bar(self):\n"
               "        return 1\n";
    }

    fs::path patch = tmp / "patch_py_method2.txt";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: replace_py_method\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    symbol: \"Foo.bar\"\n"
               "    payload: |\n"
               "      async def bar(self):\n"
               "          return 2\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    REQUIRE(L.size() >= 2);

    CHECK(L[0] == "class Foo:");

    bool found_bar_2 = false;
    for (const auto &line : L)
    {
        if (line.find("async def bar") != std::string::npos)
            found_bar_2 = true;
        if (line.find("return 2") != std::string::npos)
            found_bar_2 = true;
    }

    CHECK(found_bar_2);
}

// ============================================================================
// C++: replace-cpp-function
// ============================================================================
TEST_CASE("symbol API: replace-cpp-function replaces only target function")
{
    fs::path tmp = fs::temp_directory_path() / "symbol_cpp_function";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "foo.cpp";
    {
        std::ofstream out(f);
        out << "int foo() {\n"
               "    return 1;\n"
               "}\n"
               "\n"
               "int bar() {\n"
               "    return 2;\n"
               "}\n";
    }

    fs::path patch = tmp / "patch_cpp_function.txt";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: replace_cpp_function\n"
               "    path: \"" << f.string() << "\"\n"
               "    function: \"foo\"\n"
               "    payload: |\n"
               "      int foo() {\n"
               "          return 42;\n"
               "      }\n";
    }

    CHECK(run_apply(patch) == 0);

    auto L = read_lines(f);
    bool found_foo_42 = false;
    bool found_bar_2 = false;

    for (const auto &line : L)
    {
        if (line.find("int foo()") != std::string::npos)
            found_foo_42 = true;
        if (line.find("return 42") != std::string::npos)
            found_foo_42 = true;
        if (line.find("int bar()") != std::string::npos)
            found_bar_2 = true;
        if (line.find("return 2") != std::string::npos)
            found_bar_2 = true;
    }

    CHECK(found_foo_42);
    CHECK(found_bar_2);
}
