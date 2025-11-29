#include "guard/guard.h"
#include "apply.h"
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
    std::string arg1 = "--quiet";
    std::string arg2 = patch.string();
    // храним строки в живом виде
    std::vector<std::string> args = {arg0, arg1, arg2};

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
        out << "operations:\n"
               "  - op: insert_after_text\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    marker: \"LINE2\"\n"
               "    payload: |\n"
               "      AFTER\n"
               "      TEXT\n";
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
        out << "operations:\n"
               "  - op: insert_before_text\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    marker: \"BBB\"\n"
               "    payload: |\n"
               "      X\n"
               "      Y\n";
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

TEST_CASE("apply_chunk_main: insert-text-after synonym")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_insert_text_after";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "a.txt";
    {
        std::ofstream out(f);
        out << "ONE\n"
               "TWO\n"
               "THREE\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: insert-text-after\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    marker: \"TWO\"\n"
               "    payload: |\n"
               "      X\n"
               "      Y\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 5);
    CHECK(lines[0] == "ONE");
    CHECK(lines[1] == "TWO");
    CHECK(lines[2] == "X");
    CHECK(lines[3] == "Y");
    CHECK(lines[4] == "THREE");
}

TEST_CASE("apply_chunk_main: insert_text_before synonym")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_insert_text_before";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "b.txt";
    {
        std::ofstream out(f);
        out << "FIRST\n"
               "SECOND\n"
               "THIRD\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: insert_text_before\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    marker: \"SECOND\"\n"
               "    payload: |\n"
               "      P\n"
               "      Q\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 5);
    CHECK(lines[0] == "FIRST");
    CHECK(lines[1] == "P");
    CHECK(lines[2] == "Q");
    CHECK(lines[3] == "SECOND");
    CHECK(lines[4] == "THIRD");
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
        out << "operations:\n"
               "  - op: replace_text\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    marker: \"beta\"\n"
               "    payload: |\n"
               "      BETA1\n"
               "      BETA2\n";
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
        out << "operations:\n"
               "  - op: delete_text\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    marker: |\n"
               "      two\n"
               "      three\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "one");
    CHECK(lines[1] == "four");
}

TEST_CASE("apply_chunk_main: prepend-text")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_prepend_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "prepend.txt";
    {
        std::ofstream out(f);
        out << "MIDDLE1\n"
               "MIDDLE2\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: prepend_text\n"
               "    path: \"" << f.string() << "\"\n"
               "    payload: |\n"
               "      START1\n"
               "      START2\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0] == "START1");
    CHECK(lines[1] == "START2");
    CHECK(lines[2] == "MIDDLE1");
    CHECK(lines[3] == "MIDDLE2");
}

TEST_CASE("apply_chunk_main: append-text")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_append_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "append.txt";
    {
        std::ofstream out(f);
        out << "ONE\n"
               "TWO\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: append_text\n"
               "    path: \"" << f.string() << "\"\n"
               "    payload: |\n"
               "      THREE\n"
               "      FOUR\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0] == "ONE");
    CHECK(lines[1] == "TWO");
    CHECK(lines[2] == "THREE");
    CHECK(lines[3] == "FOUR");
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
        out << "operations:\n"
               "  - op: delete_file\n"
               "    path: \"" << f.string()
            << "\"\n"
               "  - op: create_file\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    payload: |\n"
               "      NEW1\n"
               "      NEW2\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    REQUIRE(fs::exists(f));

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "NEW1");
    CHECK(lines[1] == "NEW2");
}

TEST_CASE("apply_chunk_main: marker ignores C++ comments when language is set")
{
	fs::path tmp = fs::temp_directory_path() / "chunk_test_marker_cpp_comments";
	fs::remove_all(tmp);
	fs::create_directories(tmp);

	fs::path f = tmp / "code.cpp";
	{
		std::ofstream out(f);
		out << "int value() const {\n"
		       "    int x = 1; // old value\n"
		       "    // comment only\n"
		       "    return x; // old return\n"
		       "}\n";
	}

	fs::path patch = tmp / "patch.txt";
	{
		std::ofstream out(patch);
		out << "language: c++\n"
		       "operations:\n"
		       "  - op: replace_text\n"
		       "    path: \"" << f.string() << "\"\n"
		       "    marker: |\n"
		       "      int x = 1;\n"
		       "      return x;\n"
		       "    payload: |\n"
		       "      int x = 42;\n"
		       "      return x;\n";
	}

	int r = run_apply(patch);
	CHECK(r == 0);
	auto lines = read_lines(f);
	REQUIRE(lines.size() == 4);
	CHECK(lines[0] == "int value() const {");
	CHECK(lines[1] == "    int x = 42;");
	CHECK(lines[2] == "    return x;");
	CHECK(lines[3] == "}");
}

TEST_CASE("apply_chunk_main: marker ignores Python comments when language is set")
{
        fs::path tmp = fs::temp_directory_path() / "chunk_test_marker_python_comments";
        fs::remove_all(tmp);
        fs::create_directories(tmp);

	fs::path f = tmp / "code.py";
	{
		std::ofstream out(f);
		out << "def value(x):\n"
		       "    y = x + 1  # old value\n"
		       "    # comment only\n"
		       "    return y  # old return\n";
	}

	fs::path patch = tmp / "patch.txt";
	{
		std::ofstream out(patch);
		out << "language: python\n"
		       "operations:\n"
		       "  - op: replace_text\n"
		       "    path: \"" << f.string() << "\"\n"
		       "    marker: |\n"
		       "      y = x + 1\n"
		       "      return y\n"
		       "    payload: |\n"
		       "      y = x + 2\n"
		       "      return y\n";
	}

	int r = run_apply(patch);
	CHECK(r == 0);
	auto lines = read_lines(f);
        REQUIRE(lines.size() == 3);
        CHECK(lines[0] == "def value(x):");
        CHECK(lines[1] == "    y = x + 2");
        CHECK(lines[2] == "    return y");
}

TEST_CASE("apply_chunk_main: replace-text on top")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_replace_text";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "c.txt";
    {
        std::ofstream out(f);
        out << "header\n"
               "beta\n"
               "gamma\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: replace_text\n"
               "    path: \"" << f.string()
            << "\"\n"
               "    marker: \"header\"\n"
               "    payload: |\n"
               "      BETA1\n"
               "      BETA2\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0] == "BETA1");
    CHECK(lines[1] == "BETA2");
    CHECK(lines[2] == "beta");
    CHECK(lines[3] == "gamma");
}

TEST_CASE("apply_chunk_main: replace section in long markdown file")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_markdown_replace";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    const std::vector<std::string> original = {
        "# Sample Project",
        "",
        "Intro line 1",
        "Intro line 2",
        "",
        "## Overview",
        "- Purpose line 1",
        "- Purpose line 2",
        "- Purpose line 3",
        "",
        "## Getting Started",
        "1. Install dependencies",
        "2. Build the project",
        "3. Run the binary",
        "4. Read the docs",
        "",
        "## Details",
        "Paragraph 1",
        "Paragraph 2",
        "Paragraph 3",
        "",
        "## FAQ",
        "Q: How to configure?",
        "A: Use config.yaml",
        "",
        "## License",
        "MIT License",
    };

    fs::path f = tmp / "README.md";
    {
        std::ofstream out(f);
        for (const auto &line : original)
            out << line << '\n';
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: replace_text\n"
               "    path: \"" << f.string() << "\"\n"
               "    marker: |\n"
               "      ## Getting Started\n"
               "      1. Install dependencies\n"
               "      2. Build the project\n"
               "      3. Run the binary\n"
               "      4. Read the docs\n"
               "    payload: |\n"
               "      ## Getting Started\n"
               "      1. Install dependencies\n"
               "      2. Build the project in release mode\n"
               "      3. Run the binary with --help\n"
               "      4. Check sample configs\n"
               "      5. File issues on the tracker\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 28);

    CHECK(lines[0] == "# Sample Project");
    CHECK(lines[5] == "## Overview");
    CHECK(lines[10] == "## Getting Started");
    CHECK(lines[11] == "1. Install dependencies");
    CHECK(lines[12] == "2. Build the project in release mode");
    CHECK(lines[13] == "3. Run the binary with --help");
    CHECK(lines[14] == "4. Check sample configs");
    CHECK(lines[15] == "5. File issues on the tracker");
    CHECK(lines[17] == "## Details");
    CHECK(lines.back() == "MIT License");
}

TEST_CASE("apply_chunk_main: insert section into long markdown file")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_markdown_insert";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    const std::vector<std::string> original = {
        "# Sample Project",
        "",
        "Intro line 1",
        "Intro line 2",
        "",
        "## Overview",
        "- Purpose line 1",
        "- Purpose line 2",
        "- Purpose line 3",
        "",
        "## Getting Started",
        "1. Install dependencies",
        "2. Build the project",
        "3. Run the binary",
        "4. Read the docs",
        "",
        "## Details",
        "Paragraph 1",
        "Paragraph 2",
        "Paragraph 3",
        "",
        "## FAQ",
        "Q: How to configure?",
        "A: Use config.yaml",
        "",
        "## License",
        "MIT License",
    };

    fs::path f = tmp / "README.md";
    {
        std::ofstream out(f);
        for (const auto &line : original)
            out << line << '\n';
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: insert_after_text\n"
               "    path: \"" << f.string() << "\"\n"
               "    marker: |\n"
               "      ## FAQ\n"
               "      Q: How to configure?\n"
               "      A: Use config.yaml\n"
               "    payload: |\n"
               "      ## Contributing\n"
               "      - Fork the repository\n"
               "      - Add tests for every change\n"
               "      - Run chunk on updated docs\n"
               "      \n"
               "      ## Support\n"
               "      For help, open an issue on the tracker\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 34);

    CHECK(lines[21] == "## FAQ");
    CHECK(lines[22] == "Q: How to configure?");
    CHECK(lines[23] == "A: Use config.yaml");

    CHECK(lines[24] == "## Contributing");
    CHECK(lines[25] == "- Fork the repository");
    CHECK(lines[26] == "- Add tests for every change");
    CHECK(lines[27] == "- Run chunk on updated docs");
    CHECK(lines[28] == "");
    CHECK(lines[29] == "## Support");
    CHECK(lines[30] == "For help, open an issue on the tracker");

    CHECK(lines[32] == "## License");
    CHECK(lines[33] == "MIT License");
}

TEST_CASE("apply_chunk_main: replace_c_style_block replaces single function body")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_replace_c_style_block_simple";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "code.cpp";
    {
        std::ofstream out(f);
        out << "int foo()\n"
               "{\n"
               "    return 1;\n"
               "}\n"
               "\n"
               "int bar()\n"
               "{\n"
               "    return 2;\n"
               "}\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: replace_c_style_block\n"
               "    path: \"" << f.string() << "\"\n"
               "    marker: |\n"
               "      int foo()\n"
               "      {\n"
               "    payload: |\n"
               "      int foo()\n"
               "      {\n"
               "          return 42;\n"
               "      }\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 9);
    CHECK(lines[0] == "int foo()");
    CHECK(lines[1] == "{");
    CHECK(lines[2].find("return 42;") != std::string::npos);
    CHECK(lines[3] == "}");
    CHECK(lines[4] == "");
    CHECK(lines[5] == "int bar()");
    CHECK(lines[6] == "{");
    CHECK(lines[7].find("return 2;") != std::string::npos);
    CHECK(lines[8] == "}");
}

TEST_CASE("apply_chunk_main: replace_c_style_block handles nested braces and comments")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_replace_c_style_block_nested";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "code_nested.cpp";
    {
        std::ofstream out(f);
        out << "void foo()\n"
               "{\n"
               "    int x = 0;\n"
               "    if (x) {\n"
               "        x = 1;\n"
               "    }\n"
               "    std::string s = \"{not a brace}\";\n"
               "    // comment with }\n"
               "    /* block { comment */\n"
               "    return x;\n"
               "}\n"
               "\n"
               "int other()\n"
               "{\n"
               "    return 0;\n"
               "}\n";
    }

    fs::path patch = tmp / "patch.yml";
    {
        std::ofstream out(patch);
        out << "operations:\n"
               "  - op: replace_c_style_block\n"
               "    path: \"" << f.string() << "\"\n"
               "    marker: |\n"
               "      void foo()\n"
               "      {\n"
               "    payload: |\n"
               "      void foo()\n"
               "      {\n"
               "          int y = 42;\n"
               "          if (y > 0) {\n"
               "              y += 1;\n"
               "          }\n"
               "      }\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);

    bool has_y_decl = false;
    bool has_old_x_decl = false;
    bool has_other_function = false;
    for (const auto &line : lines)
    {
        if (line.find("int y = 42;") != std::string::npos)
            has_y_decl = true;
        if (line.find("int x = 0;") != std::string::npos)
            has_old_x_decl = true;
        if (line == "int other()")
            has_other_function = true;
    }
    CHECK(has_y_decl);
    CHECK_FALSE(has_old_x_decl);
    CHECK(has_other_function);
}

TEST_CASE("apply_chunk_main: replace-py-block replaces python block by marker")
{
    fs::path tmp = fs::temp_directory_path() / "chunk_test_replace_py_block";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    fs::path f = tmp / "main.py";
    {
        std::ofstream out(f);
        out << "# header\n"
               "if __name__ == \"__main__\":\n"
               "    print(\"OLD1\")\n"
               "    print(\"OLD2\")\n"
               "print(\"AFTER\")\n";
    }

    fs::path patch = tmp / "patch.txt";
    {
        std::ofstream out(patch);
        out << "language: python\n"
               "operations:\n"
               "  - op: replace_py_block\n"
               "    path: \"" << f.string() << "\"\n"
               "    marker: |\n"
               "      if __name__ == \"__main__\":\n"
               "    payload: |\n"
               "      if __name__ == \"__main__\":\n"
               "        print(\"NEW1\")\n"
               "        print(\"NEW2\")\n";
    }

    int r = run_apply(patch);
    CHECK(r == 0);

    auto lines = read_lines(f);
    REQUIRE(lines.size() == 5);
    CHECK(lines[0] == "# header");
    CHECK(lines[1] == "if __name__ == \"__main__\":");
    CHECK(lines[2] == "  print(\"NEW1\")");
    CHECK(lines[3] == "  print(\"NEW2\")");
    CHECK(lines[4] == "print(\"AFTER\")");
}
