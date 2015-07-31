#include "catch.hpp"

#include <bent/internal/dynamic_constructor.hpp>
#include "components/unko.hpp"

TEST_CASE("DynamicConstructor well works", "[dynamic_constructor]")
{
    bent::DynamicConstructor<unko> dctor;
    std::allocator<unko> alloc;
    auto p = alloc.allocate(1);
    unko u;
    dctor.CopyConstruct(p, &u);
    REQUIRE(p->state == unko::COPY_CONSTRUCTED);
    dctor.Destroy(p);
    REQUIRE(p->state == unko::DESTRUCTED);
    dctor.MoveConstruct(p, &u);
    REQUIRE(p->state == unko::MOVE_CONSTRUCTED);
    dctor.Destroy(p);
    REQUIRE(p->state == unko::DESTRUCTED);
}
