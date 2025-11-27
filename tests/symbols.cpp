#include "symbols.h"
#include "doctest/doctest.h"

#include <string>

TEST_CASE("CppSymbolFinder: finds simple class definition")
{
    std::string src = "class Foo {\n"
                      "public:\n"
                      "    void bar();\n"
                      "};\n"
                      "\n"
                      "class Bar {};\n";

    CppSymbolFinder finder(src);
    Region r;

    CHECK(finder.find_class("Foo", r));
    CHECK(r.start_line == 0);
    CHECK(r.end_line == 3);

    // Другой класс тоже должен находиться
    CHECK(finder.find_class("Bar", r));
}

TEST_CASE("CppSymbolFinder: ignores forward declaration")
{
    std::string src = "class Foo;\n"
                      "\n"
                      "class Foo {\n"
                      "public:\n"
                      "    void bar();\n"
                      "};\n";

    CppSymbolFinder finder(src);
    Region r;

    CHECK(finder.find_class("Foo", r));
    CHECK(r.start_line == 2);
    CHECK(r.end_line == 5);
}

TEST_CASE("CppSymbolFinder: finds method declarations inside class")
{
    std::string src = "class Foo {\n"
                      "public:\n"
                      "    int bar(int x) const;\n"
                      "    void baz();\n"
                      "};\n";

    CppSymbolFinder finder(src);

    Region r1;
    CHECK(finder.find_method("Foo", "bar", r1));
    CHECK(r1.start_line == 2);
    CHECK(r1.end_line == 2);

    Region r2;
    CHECK(finder.find_method("Foo", "baz", r2));
    CHECK(r2.start_line == 3);
    CHECK(r2.end_line == 3);
}

TEST_CASE("CppSymbolFinder: finds inline method definition")
{
    std::string src = "class Foo {\n"
                      "public:\n"
                      "    int bar(int x) const {\n"
                      "        return x + 1;\n"
                      "    }\n"
                      "};\n";

    CppSymbolFinder finder(src);
    Region r;

    CHECK(finder.find_method("Foo", "bar", r));
    CHECK(r.start_line == 2);
    CHECK(r.end_line == 4);
}

TEST_CASE("CppSymbolFinder: method not found returns false")
{
    std::string src = "class Foo {\n"
                      "public:\n"
                      "    void bar();\n"
                      "};\n";

    CppSymbolFinder finder(src);
    Region r;

    CHECK_FALSE(finder.find_method("Foo", "baz", r));
}

TEST_CASE("CppSymbolFinder: complex file with multiple classes and comments")
{
    std::string src = "#pragma once\n"                                  // 0
                      "\n"                                              // 1
                      "// forward decl\n"                               // 2
                      "class Forward;\n"                                // 3
                      "\n"                                              // 4
                      "/* comment with class Fake { */\n"               // 5
                      "\n"                                              // 6
                      "class First {\n"                                 // 7
                      "public:\n"                                       // 8
                      "    // method comment\n"                         // 9
                      "    int m1(int x) const;\n"                      // 10
                      "    std::string m2() const {\n"                  // 11
                      "        return \"class Fake {\"; // in string\n" // 12
                      "    }\n"                                         // 13
                      "};\n"                                            // 14
                      "\n"                                              // 15
                      "// Another class\n"                              // 16
                      "struct Second {\n"                               // 17
                      "    void go();\n"                                // 18
                      "};\n";                                           // 19

    CppSymbolFinder finder(src);
    Region r;

    // forward declaration не должен считаться определением
    CHECK_FALSE(finder.find_class("Forward", r));

    // Первый класс
    CHECK(finder.find_class("First", r));
    CHECK(r.start_line == 7);
    CHECK(r.end_line == 14);

    // Второй класс
    CHECK(finder.find_class("Second", r));
    CHECK(r.start_line == 17);
    CHECK(r.end_line == 19);

    // Методы первого класса
    Region m1;
    CHECK(finder.find_method("First", "m1", m1));
    CHECK(m1.start_line == 10);
    CHECK(m1.end_line == 10);

    Region m2;
    CHECK(finder.find_method("First", "m2", m2));
    CHECK(m2.start_line == 11);
    CHECK(m2.end_line == 13);

    // Метод во втором классе
    Region m_go;
    CHECK(finder.find_method("Second", "go", m_go));
    CHECK(m_go.start_line == 18);
    CHECK(m_go.end_line == 18);

    CHECK_FALSE(finder.find_class("Fake", r));
}

TEST_CASE("CppSymbolFinder: handles extra whitespace and newlines")
{
    std::string src = "  \n"                 // 0
                      "class   Weird  \n"    // 1
                      "  : public Base\n"    // 2
                      "{\n"                  // 3
                      "public:\n"            // 4
                      "    void  run ( );\n" // 5
                      "};\n";                // 6

    CppSymbolFinder finder(src);
    Region rc;

    CHECK(finder.find_class("Weird", rc));
    CHECK(rc.start_line == 1);
    CHECK(rc.end_line == 6);

    Region rm;
    CHECK(finder.find_method("Weird", "run", rm));
    CHECK(rm.start_line == 5);
    CHECK(rm.end_line == 5);
}
