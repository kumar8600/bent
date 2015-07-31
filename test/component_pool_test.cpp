#include "catch.hpp"

#include <bent/internal/component_pool.hpp>

TEST_CASE("ComponentPool well works", "[component_pool]")
{
    bent::ComponentPool<int> pool;
    auto p1 = (int*) pool.Allocate(1);
    REQUIRE(p1 == pool.Get(1));
    REQUIRE(p1 == pool.Allocate(1));
}
