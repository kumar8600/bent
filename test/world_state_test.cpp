#include "catch.hpp"

#include <bent/internal/world_state.hpp>

#include <stdexcept>

#include <bent/component_manager.hpp>
#include "components/unko.hpp"

TEST_CASE("World state is well created", "[world_state]")
{
    bent::WorldState state;

    auto intid = bent::ComponentManager::instance().id<int>();
    auto unkoid = bent::ComponentManager::instance().id<unko>();

    SECTION("Entity creation/destroying")
    {
        auto e1 = state.AddEntity(1);
        auto e2 = state.AddEntity(2);

        REQUIRE_THROWS_AS(state.AddEntity(2), std::out_of_range);
        REQUIRE_THROWS_AS(state.AddEntity(1), std::out_of_range);
        
        REQUIRE_NOTHROW(state.RemoveEntity(e2));
        REQUIRE_THROWS_AS(state.AddEntity(2), std::out_of_range);

        REQUIRE_THROWS_AS(state.AddEntity(1), std::out_of_range);

        REQUIRE_NOTHROW(state.RemoveEntity(e1));

        state.AddEntity(1000);
    }

    SECTION("Component attaching/detaching")
    {
        auto e1 = state.AddEntity(1);
        REQUIRE_NOTHROW(state.AddComponent<unko>(e1, unkoid));
        REQUIRE_NOTHROW(state.AddComponent<int>(e1, intid, 0xdeadbeef));

        REQUIRE(((unko*) state.GetComponent(e1, unkoid))->state == unko::DEFAULT_CONSTRUCTED);
        REQUIRE(*(int*) state.GetComponent(e1, intid) == 0xdeadbeef);

        auto e2 = state.AddEntity(2);
        REQUIRE_NOTHROW(state.AddComponent<int>(e2, intid, 0xfee1dead));
        REQUIRE(*(int*) state.GetComponent(e2, intid) == 0xfee1dead);
        
        REQUIRE(*(int*) state.GetComponent(e1, intid) == 0xdeadbeef);

        REQUIRE_NOTHROW(state.RemoveComponent(e1, unkoid));
        REQUIRE_NOTHROW(state.RemoveComponent(e1, intid));

        unko u;
        state.AddComponent<unko>(e1, unkoid, u);
        REQUIRE(((unko*) state.GetComponent(e1, unkoid))->state == unko::COPY_CONSTRUCTED);
        state.RemoveComponent(e1, unkoid);

        state.AddComponent<unko>(e1, unkoid, std::move(u));
        REQUIRE(((unko*) state.GetComponent(e1, unkoid))->state == unko::MOVE_CONSTRUCTED);
        state.RemoveComponent(e1, unkoid);

        state.AddComponentFrom(e1, unkoid, &u);
        REQUIRE(((unko*) state.GetComponent(e1, unkoid))->state == unko::COPY_CONSTRUCTED);
        state.RemoveComponent(e1, unkoid);

        state.AddComponentFromMove(e1, unkoid, &u);
        REQUIRE(((unko*) state.GetComponent(e1, unkoid))->state == unko::MOVE_CONSTRUCTED);
        state.RemoveComponent(e1, unkoid);

        auto e3 = state.AddEntity(3);
        int x = 0xfacefeed;
        state.AddComponent<int>(e3, intid, x);
        REQUIRE_THROWS_AS(state.AddComponent<int>(e3, intid, x), std::out_of_range);
        REQUIRE_THROWS_AS(state.AddComponentFrom(e3, intid, &x), std::out_of_range);
        REQUIRE_THROWS_AS(state.AddComponentFromMove(e3, intid, &x), std::out_of_range);
    }

    SECTION("Entity lookup")
    {
        auto e1 = state.AddEntity(1);
        state.AddComponent<int>(e1, intid, 0xdeadbeef);
        auto e2 = state.AddEntity(2);
        state.AddComponent<int>(e2, intid, 0xfee1dead);

        REQUIRE_FALSE(state.FindEntity(0));
        REQUIRE(state.FindEntity(1) == e1);
        REQUIRE(state.FindEntity(2) == e2);
        REQUIRE_FALSE(state.FindEntity(3));
        state.RemoveEntity(e1);
        REQUIRE_FALSE(state.FindEntity(1));
        REQUIRE(state.FindEntity(2) == e2);
        state.RemoveEntity(e2);
        REQUIRE_FALSE(state.FindEntity(1));
        REQUIRE_FALSE(state.FindEntity(2));
    }

    SECTION("Garbage Collection")
    {
        REQUIRE(state.version() == 0);

        auto e1 = state.AddEntity(1);
        state.AddComponent<int>(e1, intid, 0xdeadbeef);
        auto e2 = state.AddEntity(2);
        state.AddComponent<int>(e2, intid, 0xfee1dead);
        auto e3 = state.AddEntity(3);
        state.AddComponent<int>(e3, intid, 0xfacefeed);

        state.RemoveEntity(e2);

        REQUIRE_FALSE(state.IsEntityValid(e2));

        auto e1i = (int*) state.GetComponent(state.FindEntity(1), intid);
        auto e3i = (int*) state.GetComponent(state.FindEntity(3), intid);

        SECTION("by copy")
        {
            bent::WorldState copied_state(state);

            auto ce1i = (int*) copied_state.GetComponent(copied_state.FindEntity(1), intid);
            REQUIRE(*ce1i == 0xdeadbeef);
            REQUIRE(ce1i != e1i);

            auto ce3i = (int*) copied_state.GetComponent(copied_state.FindEntity(3), intid);
            REQUIRE(*ce3i == 0xfacefeed);
            REQUIRE(ce3i != e3i);

            REQUIRE(ce1i + 1 == ce3i);

            REQUIRE(state.version() == 0);
            REQUIRE(copied_state.version() == 1);
        }

        SECTION("by method")
        {
            state.CollectGarbage();

            auto ce1i = (int*) state.GetComponent(state.FindEntity(1), intid);
            REQUIRE(*ce1i == 0xdeadbeef);
            REQUIRE(ce1i != e1i);

            auto ce3i = (int*) state.GetComponent(state.FindEntity(3), intid);
            REQUIRE(*ce3i == 0xfacefeed);
            REQUIRE(ce3i != e3i);

            REQUIRE(ce1i + 1 == ce3i);

            REQUIRE(state.version() == 1);
        }
    }
}
