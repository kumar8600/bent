#include "catch.hpp"

#include <bent/entity_handle.hpp>
#include <bent/world.hpp>

#include "components/position.hpp"
#include "components/unko.hpp"

TEST_CASE("Entity handles are well works", "[entity_handle]")
{
    bent::World world;
    
    SECTION("creation/destroying")
    {
        auto e1 = world.Create();
        REQUIRE(e1.id() == 1);
        auto e2 = world.Create();
        REQUIRE(e2.id() == 2);

        REQUIRE(e1.valid());
        REQUIRE(e2.valid());

        REQUIRE_NOTHROW(e1.Destroy());
        REQUIRE_FALSE(e1.valid());
        REQUIRE(e2.valid());

        // garbage collection does not invalidate entity handles.
        world.CollectGarbage();
        REQUIRE(e2.valid());

        REQUIRE_NOTHROW(e2.Destroy());
        REQUIRE_FALSE(e1.valid());
        REQUIRE_FALSE(e2.valid());

        auto e3 = world.Create();
        REQUIRE(e3.id() == 3);
    }
    
    SECTION("component attaching/detaching")
    {
        auto e1 = world.Create();
        auto e2 = world.Create();

        REQUIRE_NOTHROW(e1.Add<Position>(1.0f, 2.0f));
        REQUIRE_THROWS_AS(e1.Add<Position>(1.0f, 2.0f), std::out_of_range);
        REQUIRE(e1.Get<Position>()->x == 1.0f);
        REQUIRE(e1.Get<Position>()->y == 2.0f);

        e2.Add<Position>(2.0f, 3.0f);
        REQUIRE(e2.Get<Position>()->x == 2.0f);
        REQUIRE(e2.Get<Position>()->y == 3.0f);

        REQUIRE(e1.Get<Position>()->x == 1.0f);
        REQUIRE(e1.Get<Position>()->y == 2.0f);

        REQUIRE_NOTHROW(e1.Remove<Position>());
        REQUIRE_THROWS_AS(e1.Remove<Position>(), std::out_of_range);
        REQUIRE(e1.Get<Position>() == nullptr);
        e1.Destroy();
        REQUIRE_THROWS_AS(e1.Add<Position>(1.0f, 2.0f), std::logic_error);
        REQUIRE_THROWS_AS(e1.Get<Position>(), std::logic_error);
        REQUIRE_THROWS_AS(e1.Remove<Position>(), std::logic_error);

        unko u;
        auto e3 = world.Create();

        e3.AddFrom(u);
        REQUIRE(e3.Get<unko>()->state == unko::COPY_CONSTRUCTED);
        e3.Remove<unko>();

        e3.AddFrom(std::move(u));
        REQUIRE(e3.Get<unko>()->state == unko::MOVE_CONSTRUCTED);
        e3.Remove<unko>();

        bent::RegisterComponent<unko>("unko");

        e3.AddFrom("unko", &u);
        REQUIRE(((unko*) e3.Get("unko"))->state == unko::COPY_CONSTRUCTED);
        e3.Remove("unko");

        e3.AddFromMove("unko", &u);
        REQUIRE(((unko*) e3.Get("unko"))->state == unko::MOVE_CONSTRUCTED);
        e3.Remove("unko");
    }

    SECTION("operators")
    {
        auto e1 = world.Create();
        auto e2 = world.Create();

        REQUIRE(e1 == world.Get(1));
        REQUIRE(e1 != e2);

        REQUIRE(e1 == e1);
        REQUIRE(e2 == e2);
    }
}
