#include "catch.hpp"

#include <bent/internal/entity_id_manager.hpp>

TEST_CASE("Entity id is well generated", "[entity_id_manager]")
{
    bent::EntityIdManager manager;

    REQUIRE(manager.Create() == 1);
    REQUIRE(manager.Create() == 2);
    REQUIRE(manager.Create() == 3);
    REQUIRE(manager.Create() == 4);
    REQUIRE(manager.Create() == 5);
    REQUIRE_NOTHROW(manager.Destroy(5));
    REQUIRE(manager.Create() == 6);
}
