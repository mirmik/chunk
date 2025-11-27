#include "doctest/doctest.h"
#include <yaml/yaml.h>
#include <cmath>
#include <limits>
#include <string>
#include <stdexcept>

TEST_CASE("YAML parser reads flow collections")
{
    const char *src =
        "flow: [1, 2, {foo: true, bar: [alpha, beta]}]\n";

    nos::trent root = nos::yaml::parse(src);
    auto &dict = root.as_dict();
    auto &flow = dict.at("flow").as_list();

    REQUIRE(flow.size() == 3);
    CHECK(flow[0].as_numer() == doctest::Approx(1));
    CHECK(flow[1].as_numer() == doctest::Approx(2));

    auto &map = flow[2].as_dict();
    CHECK(map.at("foo").as_bool() == true);

    auto &inner = map.at("bar").as_list();
    REQUIRE(inner.size() == 2);
    CHECK(inner[0].as_string() == "alpha");
    CHECK(inner[1].as_string() == "beta");
}

TEST_CASE("YAML parser handles block scalars and folding")
{
    const char *src =
        "literal: |\n"
        "  line1\n"
        "  line2\n"
        "folded: >-\n"
        "  a\n"
        "  b\n"
        "\n"
        "  c\n";

    nos::trent root = nos::yaml::parse(src);
    auto &dict = root.as_dict();

    CHECK(dict.at("literal").as_string() == "line1\nline2\n");
    CHECK(dict.at("folded").as_string() == "a b\nc");
}

TEST_CASE("YAML parser understands YAML 1.2 scalars")
{
    const char *src =
        "v1: 1_000\n"
        "v2: .inf\n"
        "v3: -.inf\n"
        "v4: .nan\n";

    nos::trent root = nos::yaml::parse(src);
    auto &dict = root.as_dict();

    CHECK(dict.at("v1").as_numer() == doctest::Approx(1000));
    CHECK(std::isinf(dict.at("v2").as_numer()));
    CHECK(dict.at("v3").as_numer() < 0);
    CHECK(std::isnan(dict.at("v4").as_numer()));
}

TEST_CASE("YAML parser reports flow errors with location")
{
    const char *src = "flow: [1, 2\n";

    try
    {
        (void)nos::yaml::parse(src);
        FAIL("parse must throw");
    }
    catch (const nos::yaml::parse_error &err)
    {
        CHECK(err.line() == 1);
        CHECK(err.column() != 0);
        CHECK(std::string(err.what()).find("unterminated flow") !=
              std::string::npos);
    }
}

TEST_CASE("YAML parser handles document markers and comments")
{
    const char *src =
        "---\n"
        "name: test # comment\n"
        "list: [a, b] # trailing\n"
        "...\n";

    nos::trent root = nos::yaml::parse(src);
    auto &dict = root.as_dict();
    CHECK(dict.at("name").as_string() == "test");
    auto &list = dict.at("list").as_list();
    REQUIRE(list.size() == 2);
    CHECK(list[0].as_string() == "a");
    CHECK(list[1].as_string() == "b");
}

TEST_CASE("YAML parser respects quoted scalars and escapes")
{
    const char *src =
        "double: \"line\\nwith\\\\slash\\\"quote\\u0041\"\n"
        "single: 'it''s fine'\n"
        "colon: \"a: b\"\n";

    auto root = nos::yaml::parse(src).as_dict();
    CHECK(root.at("double").as_string() == "line\nwith\\slash\"quoteA");
    CHECK(root.at("single").as_string() == "it's fine");
    CHECK(root.at("colon").as_string() == "a: b");
}

TEST_CASE("YAML parser handles flow collections across lines")
{
    const char *src =
        "flow:\n"
        "  [1,\n"
        "   2,\n"
        "   {key: [10,\n"
        "         20]}]\n";

    auto root = nos::yaml::parse(src).as_dict();
    auto &flow = root.at("flow").as_list();
    REQUIRE(flow.size() == 3);
    CHECK(flow[2].as_dict().at("key").as_list().at(1).as_numer() ==
          doctest::Approx(20));
}

TEST_CASE("YAML parser handles literal, folded and chomping styles")
{
    const char *src =
        "literal: |+\n"
        "  one\n"
        "  two\n"
        "\n"
        "folded: >-\n"
        "  a\n"
        "  b\n"
        "\n"
        "  c\n"
        "strip: |-\n"
        "  keep\n"
        "  text\n"
        "\n";

    auto dict = nos::yaml::parse(src).as_dict();
    CHECK(dict.at("literal").as_string() == "one\ntwo\n\n");
    CHECK(dict.at("folded").as_string() == "a b\nc");
    CHECK(dict.at("strip").as_string() == "keep\ntext");
}

TEST_CASE("YAML parser recognizes YAML 1.2 numeric/boolean/null forms")
{
    const char *src =
        "u: 1_234_567\n"
        "pinf: +.inf\n"
        "ninf: -.inf\n"
        "nanv: .nan\n"
        "nulls: [null, ~]\n"
        "bools: [TRUE, false]\n";

    auto dict = nos::yaml::parse(src).as_dict();
    CHECK(dict.at("u").as_numer() == doctest::Approx(1234567));
    CHECK(std::isinf(dict.at("pinf").as_numer()));
    CHECK(dict.at("ninf").as_numer() < 0);
    CHECK(std::isnan(dict.at("nanv").as_numer()));
    auto &nulls = dict.at("nulls").as_list();
    CHECK(nulls[0].is_nil());
    CHECK(nulls[1].is_nil());
    auto &bools = dict.at("bools").as_list();
    CHECK(bools[0].as_bool());
    CHECK_FALSE(bools[1].as_bool());
}

TEST_CASE("YAML parser treats colon without whitespace as plain scalar")
{
    const char *src =
        "text: foo:bar\n";
    auto dict = nos::yaml::parse(src).as_dict();
    CHECK(dict.at("text").as_string() == "foo:bar");
}

TEST_CASE("YAML parser errors on bad indentation with location")
{
    const char *src =
        "a:\n"
        "  - 1\n"
        " - 2\n";

    CHECK_THROWS_AS(nos::yaml::parse(src), nos::yaml::parse_error);
    try
    {
        (void)nos::yaml::parse(src);
    }
    catch (const nos::yaml::parse_error &err)
    {
        CHECK(err.line() == 3);
    }
}

TEST_CASE("YAML parser errors on empty keys")
{
    const char *src = ": value\n";
    CHECK_THROWS_AS(nos::yaml::parse(src), nos::yaml::parse_error);
}

TEST_CASE("YAML parser handles nested sequences and mappings")
{
    const char *src =
        "root:\n"
        "  - name: item1\n"
        "    values: [1, 2, 3]\n"
        "  - name: item2\n"
        "    details:\n"
        "      enabled: true\n"
        "      tags:\n"
        "        - x\n"
        "        - y\n";

    auto root = nos::yaml::parse(src).as_dict().at("root").as_list();
    REQUIRE(root.size() == 2);
    CHECK(root[0].as_dict().at("values").as_list().at(2).as_numer() ==
          doctest::Approx(3));
    CHECK(root[1].as_dict()
              .at("details")
              .as_dict()
              .at("tags")
              .as_list()
              .at(1)
              .as_string() == "y");
}

TEST_CASE("YAML parser supports flow map values on mapping lines")
{
    const char *src =
        "cfg: {a: 1, b: [2, 3]}\n";

    auto cfg = nos::yaml::parse(src).as_dict().at("cfg").as_dict();
    CHECK(cfg.at("a").as_numer() == doctest::Approx(1));
    auto &arr = cfg.at("b").as_list();
    CHECK(arr.size() == 2);
    CHECK(arr[1].as_numer() == doctest::Approx(3));
}

TEST_CASE("YAML parser honors empty block scalar when indent inferred")
{
    const char *src =
        "k: |\n"
        "  line\n"
        "\n";
    auto dict = nos::yaml::parse(src).as_dict();
    CHECK(dict.at("k").as_string() == "line\n");
}
