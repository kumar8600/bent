#include "catch.hpp"

#include <bent/world.hpp>
#include <bent/view.hpp>

struct WtPosition
{
    WtPosition(float x, float y) : x(x), y(y) {}
    float x, y;
};

struct WtVelocity
{
    WtVelocity(float x, float y) : x(x), y(y) {}
    float x, y;
};

struct WtFlag
{
};

TEST_CASE("World is good", "[world]")
{
    bent::World world;

    // world::create is tested on `entity_handle_test.cpp`
    // world::get is tested on `enitity_handle_test.cpp`

    auto e1 = world.Create();
    auto e2 = world.Create();
    auto e3 = world.Create();

    e1.Add<WtPosition>(10.0f, 20.0f);
    e2.Add<WtPosition>(15.0f, 25.0f);
    e3.Add<WtPosition>(20.0f, 30.0f);

    e1.Add<WtVelocity>(1.0f, 2.0f);

    e2.Add<WtFlag>();
    e3.Add<WtFlag>();

    SECTION("garbage collection")
    {
        e2.Destroy();

        world.CollectGarbage();

        REQUIRE(e1.Get<WtPosition>() + 1 == e3.Get<WtPosition>());
    }

    SECTION("view creation without type")
    {
        bent::RegisterComponent<WtPosition>("WtPosition");
        bent::RegisterComponent<WtVelocity>("WtVelocity");
        bent::RegisterComponent<WtFlag>("WtFlag");

        const char * argv [] = { "WtPosition", "WtVelocity" };
        auto view = world.entities_with(2, argv);
        
        auto it = view.begin();
        REQUIRE(*it == e1);
        ++it;
        REQUIRE(it == view.end());
    }
}
