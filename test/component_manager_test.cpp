#include "catch.hpp"

#include <bent/component_manager.hpp>

struct CmPosition
{
    float x, y;
};

struct CmVelocity
{
    float x, y;
};

struct CmFlag
{
};

TEST_CASE("Component type manager well works", "[component_manager]")
{
    bent::ComponentManager& manager = bent::ComponentManager::instance();

    auto size0 = manager.size();

    auto cmposition = manager.RegisterComponent<CmPosition>("cm_Position");
    REQUIRE_THROWS_AS(manager.RegisterComponent<CmPosition>("cm_Position2"), std::out_of_range);
    auto cmflag = manager.RegisterComponent<CmFlag>("cm_Flag");

    REQUIRE(manager.size() == size0 + 2);

    auto size1 = manager.size();
    auto cmvelocity = manager.id<CmVelocity>();
    REQUIRE(manager.size() == size1 + 1);

    REQUIRE(manager.id<CmVelocity>() == cmvelocity);
    
    auto size2 = manager.size();
    manager.RegisterComponent<CmVelocity>("cm_Velocity");
    REQUIRE(manager.size() == size2);

    REQUIRE(manager.id("cm_Position") == cmposition);
    REQUIRE(manager.id("cm_Velocity") == cmvelocity);
    REQUIRE(manager.id("cm_Flag") == cmflag);

    REQUIRE(manager.name(cmposition) == "cm_Position");
    REQUIRE(manager.name(cmvelocity) == "cm_Velocity");
    REQUIRE(manager.name(cmflag) == "cm_Flag");

    REQUIRE(typeid(manager.dynamic_constructor(cmposition)) == typeid(bent::DynamicConstructor<CmPosition>));
    REQUIRE(typeid(manager.dynamic_constructor(cmvelocity)) == typeid(bent::DynamicConstructor<CmVelocity>));
    REQUIRE(typeid(manager.dynamic_constructor(cmflag)) == typeid(bent::DynamicConstructor<CmFlag>));

    REQUIRE(typeid(manager.component_pool_factory(cmposition)) == typeid(bent::ComponentPoolFactory<CmPosition>));
    REQUIRE(typeid(manager.component_pool_factory(cmvelocity)) == typeid(bent::ComponentPoolFactory<CmVelocity>));
    REQUIRE(typeid(manager.component_pool_factory(cmflag)) == typeid(bent::ComponentPoolFactory<CmFlag>));
}
