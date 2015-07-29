#include "catch.hpp"

#include <bent/view.hpp>
#include <bent/world.hpp>

#include "components/position.hpp"
#include "components/velocity.hpp"

struct Flag
{
};

TEST_CASE("Entity iteration by View", "[view]")
{
    bent::World world;
    auto e1 = world.Create();
    auto e2 = world.Create();
    auto e3 = world.Create();

    e1.Add<Position>(10.0f, 20.0f);
    e3.Add<Position>(20.0f, 30.0f);

    e1.Add<Velocity>(1.0f, 2.0f);

    e2.Add<Flag>();
    e3.Add<Flag>();

    SECTION("without query")
    {
        auto view = world.entities_with<>();
        auto it = view.begin();
        REQUIRE(*it == e1);
        ++it;
        REQUIRE(*it == e2);
        ++it;
        REQUIRE(*it == e3);
        ++it;
        REQUIRE(it == view.end());
    }

    SECTION("with unary query")
    {
        {
            auto view = world.entities_with<Position>();
            auto it = view.begin();
            REQUIRE(*it == e1);
            ++it;
            REQUIRE(*it == e3);
            ++it;
            REQUIRE(it == view.end());
        }
        {
            auto view = world.entities_with<Velocity>();
            auto it = view.begin();
            REQUIRE(*it == e1);
            ++it;
            REQUIRE(it == view.end());
        }
        {
            auto view = world.entities_with<Flag>();
            auto it = view.begin();
            REQUIRE(*it == e2);
            ++it;
            REQUIRE(*it == e3);
            ++it;
            REQUIRE(it == view.end());
        }
    }

    SECTION("with compound query")
    {
        {
            auto view = world.entities_with<Position, Velocity>();
            auto it = view.begin();
            REQUIRE(*it == e1);
            ++it;
            REQUIRE(it == view.end());
        }
        {
            auto view = world.entities_with<Position, Flag>();
            auto it = view.begin();
            REQUIRE(*it == e3);
            ++it;
            REQUIRE(it == view.end());
        }
        {
            auto view = world.entities_with<Position, Velocity, Flag>();
            auto it = view.begin();
            REQUIRE(it == view.end());
        }
    }
}
