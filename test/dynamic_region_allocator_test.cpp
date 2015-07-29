#include "catch.hpp"

#include <bent/internal/dynamic_region_allocator.hpp>

#include <memory>
#include <string>

#include "components/unko.hpp"

TEST_CASE("Dynamic allocator well works", "[dynamic_allocator]")
{
    kumar::Region region;
    SECTION("int pointer")
    {
        kumar::DynamicRegionAllocator<int> alloc;

        int* p0 = (int*) alloc.Allocate(1, region);
        auto src = std::make_unique<int>(0xdeadbeef);
        alloc.CopyConstruct(p0, src.get());
        REQUIRE(*p0 == 0xdeadbeef);
        int* p1 = (int*) alloc.Allocate(1, region);
        alloc.MoveConstruct(p1, src.get());
        REQUIRE(*p1 == 0xdeadbeef);

        REQUIRE_NOTHROW(alloc.Deallocate(p0, 1, region));
        REQUIRE_NOTHROW(alloc.Deallocate(p1, 1, region));
    }

    SECTION("struct pointer")
    {
        kumar::DynamicRegionAllocator<unko> alloc;

        unko* p0 = (unko*) alloc.Allocate(1, region);
        auto src = std::make_unique<unko>();
        alloc.CopyConstruct(p0, src.get());
        REQUIRE(p0->state == unko::COPY_CONSTRUCTED);
        unko* p1 = (unko*) alloc.Allocate(1, region);
        alloc.MoveConstruct(p1, src.get());
        REQUIRE(p1->state == unko::MOVE_CONSTRUCTED);
        alloc.Destroy(p0);
        REQUIRE(p0->state == unko::DESTRUCTED);

        REQUIRE_NOTHROW(alloc.Deallocate(p0, 1, region));
        REQUIRE_NOTHROW(alloc.Deallocate(p1, 1, region));
    }
}
