#include "catch.hpp"

#include <bent/internal/region.hpp>

TEST_CASE("Region allocates spaces", "[region]")
{
    kumar::Region region;

    SECTION("allocated address is valid")
    {
        {
            auto ptr0 = region.Allocate(1, 1, 1);
            auto ptr1 = region.Allocate(1, 1, 1);
            REQUIRE((char*) ptr1 - (char*) ptr0 == 1);
        }

        {
            auto ptr0 = region.Allocate(1, 1, 100);
            auto ptr1 = region.Allocate(1, 1, 110);
            REQUIRE((char*) ptr1 - (char*) ptr0 == 100);
        }

        {
            auto ptr0 = region.Allocate(1, 10, 1);
            auto ptr1 = region.Allocate(1, 15, 1);
            REQUIRE((char*) ptr1 - (char*) ptr0 == 10);
        }

        {
            auto ptr0 = region.Allocate(4, 1, 1);
            auto ptr1 = region.Allocate(8, 1, 1);
            REQUIRE((char*) ptr1 - (char*) ptr0 == 8);
        }

        {
            auto ptr0 = region.Allocate(4, 4, 4);
            auto ptr1 = region.Allocate(4, 4, 4);
            REQUIRE((char*) ptr1 - (char*) ptr0 == 16);
        }

        {
            auto ptr0 = region.Allocate(4, 4, 4);
            auto ptr1 = region.Allocate(8, 8, 4);
            REQUIRE((char*) ptr1 - (char*) ptr0 == 16);
        }

        {
            auto ptr0 = region.Allocate(4, 8, 3);
            auto ptr1 = region.Allocate(8, 8, 3);
            REQUIRE((char*) ptr1 - (char*) ptr0 == 24);
        }

        {
            auto ptr0 = region.Allocate(4, 4, 3);
            auto ptr1 = region.Allocate(8, 4, 3);
            REQUIRE((char*) ptr1 - (char*) ptr0 == 16);
        }
    }
}

TEST_CASE("Region allocates another space", "[region]")
{
    kumar::Region region(4);
    REQUIRE(region.size_reserved() == 4);

    SECTION("allocated address is valid")
    {
        auto ptr0 = region.Allocate(4, 4, 1);
        auto ptr1 = region.Allocate(4, 4, 1);
        REQUIRE(region.buffer_count() == 2);
    }
}

TEST_CASE("Regions move", "[region]")
{
    kumar::Region region;

    SECTION("empty region move")
    {
        kumar::Region region_m(std::move(region));
        REQUIRE(region_m.size_allocated() == 0);
    }

    SECTION("move")
    {
        std::uint32_t* p0 = (std::uint32_t*) region.Allocate(4, 4, 1);
        *p0 = 0xdeadbeef;
        region.Allocate(4, 8, 100);
        std::uint64_t* p1 = (std::uint64_t*) region.Allocate(8, 8, 1);
        *p1 = 0xfee1dead;

        kumar::Region region_m(std::move(region));
        REQUIRE_NOTHROW(*p0);
        REQUIRE_NOTHROW(*p1);
    }

    SECTION("move assignment")
    {
        std::uint32_t* p0 = (std::uint32_t*) region.Allocate(4, 4, 1);
        *p0 = 0xdeadbeef;
        region.Allocate(4, 8, 100);
        std::uint64_t* p1 = (std::uint64_t*) region.Allocate(8, 8, 1);
        *p1 = 0xfee1dead;

        kumar::Region region_m;
        REQUIRE_NOTHROW(region_m = std::move(region));
        REQUIRE_NOTHROW(*p0);
        REQUIRE_NOTHROW(*p1);
    }
}
