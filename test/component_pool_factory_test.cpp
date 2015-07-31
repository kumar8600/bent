#include "catch.hpp"

#include <bent/internal/component_pool_factory.hpp>

TEST_CASE("ComponentPoolFactory well works", "[component_pool_factory]")
{
    bent::ComponentPoolFactory<int> factory;
    REQUIRE(typeid(*factory.Create()) == typeid(bent::ComponentPool<int>));
}
