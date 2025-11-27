#include "doctest/doctest.h"
#include <yaml/yaml.h>
#include <cmath>
#include <string>

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
