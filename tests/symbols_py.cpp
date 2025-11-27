#include "doctest/doctest.h"
#include "symbols.h"

#include <string>

TEST_CASE("PythonSymbolFinder: simple class and methods")
{
    std::string src = "class Foo:\n"            // 0
                      "    def bar(self):\n"    // 1
                      "        pass\n"          // 2
                      "\n"                      // 3
                      "    def baz(self, x):\n" // 4
                      "        return x + 1\n"; // 5

    PythonSymbolFinder finder(src);
    Region r_class;

    CHECK(finder.find_class("Foo", r_class));
    CHECK(r_class.start_line == 0);
    CHECK(r_class.end_line == 5);

    Region r_bar;
    CHECK(finder.find_method("Foo", "bar", r_bar));
    CHECK(r_bar.start_line == 1);
    CHECK(r_bar.end_line == 2);

    Region r_baz;
    CHECK(finder.find_method("Foo", "baz", r_baz));
    CHECK(r_baz.start_line == 4);
    CHECK(r_baz.end_line == 5);
}

TEST_CASE("PythonSymbolFinder: ignores top-level functions")
{
    std::string src = "def foo():\n"         // 0
                      "    pass\n"           // 1
                      "\n"                   // 2
                      "class Foo:\n"         // 3
                      "    def foo(self):\n" // 4
                      "        pass\n";      // 5

    PythonSymbolFinder finder(src);
    Region r_class;

    CHECK(finder.find_class("Foo", r_class));
    CHECK(r_class.start_line == 3);
    CHECK(r_class.end_line == 5);

    Region r_method;
    CHECK(finder.find_method("Foo", "foo", r_method));
    CHECK(r_method.start_line == 4);
    CHECK(r_method.end_line == 5);
}

TEST_CASE("PythonSymbolFinder: decorated methods")
{
    std::string src = "class Foo:\n"         // 0
                      "    @decorator\n"     // 1
                      "    def bar(self):\n" // 2
                      "        pass\n";      // 3

    PythonSymbolFinder finder(src);
    Region r_method;

    CHECK(finder.find_method("Foo", "bar", r_method));
    CHECK(r_method.start_line == 1); // вместе с декоратором
    CHECK(r_method.end_line == 3);
}

TEST_CASE("PythonSymbolFinder: nested class and methods")
{
    std::string src = "class Outer:\n"         // 0
                      "    def a(self):\n"     // 1
                      "        pass\n"         // 2
                      "    class Inner:\n"     // 3
                      "        def b(self):\n" // 4
                      "            pass\n";    // 5

    PythonSymbolFinder finder(src);

    Region r_outer;
    CHECK(finder.find_class("Outer", r_outer));
    CHECK(r_outer.start_line == 0);
    CHECK(r_outer.end_line == 5);

    Region r_a;
    CHECK(finder.find_method("Outer", "a", r_a));
    CHECK(r_a.start_line == 1);
    CHECK(r_a.end_line == 2);

    Region r_inner_class;
    CHECK(finder.find_class("Inner", r_inner_class));
    CHECK(r_inner_class.start_line == 3);
    CHECK(r_inner_class.end_line == 5);

    Region r_b;
    CHECK(finder.find_method("Inner", "b", r_b));
    CHECK(r_b.start_line == 4);
    CHECK(r_b.end_line == 5);
}

TEST_CASE("PythonSymbolFinder: async methods")
{
    std::string src = "class Foo:\n"                 // 0
                      "    async def bar(self):\n"   // 1
                      "        await something()\n"; // 2

    PythonSymbolFinder finder(src);

    Region r_class;
    CHECK(finder.find_class("Foo", r_class));
    CHECK(r_class.start_line == 0);
    CHECK(r_class.end_line == 2);

    Region r_bar;
    CHECK(finder.find_method("Foo", "bar", r_bar));
    CHECK(r_bar.start_line == 1);
    CHECK(r_bar.end_line == 2);
}

TEST_CASE("PythonSymbolFinder: whitespace, comments and docstrings")
{
    std::string src = "#!/usr/bin/env python3\n"                // 0
                      "\n"                                      // 1
                      "class   Weird  (  Base  ):\n"            // 2
                      "    \\\"\\\"\\\"docstring\\\"\\\"\\\"\n" // 3
                      "    # comment inside class\n"            // 4
                      "    def   run ( self,  x ):\n"           // 5
                      "        # inner comment\n"               // 6
                      "        return x + 1\n"                  // 7
                      "\n"                                      // 8
                      "def run(x):\n"                           // 9
                      "    return x\n";                         // 10

    PythonSymbolFinder finder(src);

    Region r_class;
    CHECK(finder.find_class("Weird", r_class));
    CHECK(r_class.start_line == 2);
    CHECK(r_class.end_line == 7);

    Region r_run;
    CHECK(finder.find_method("Weird", "run", r_run));
    CHECK(r_run.start_line == 5);
    CHECK(r_run.end_line == 7);
}
