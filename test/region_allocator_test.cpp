#include "catch.hpp"

#include <bent/internal/region_allocator.hpp>

#include <vector>

TEST_CASE("Region allocator well works", "[region_allocator]")
{
    kumar::Region region;
    kumar::RegionAllocator<int> allocator(region);
    {
        int* ptr0 = allocator.allocate(1);
        int* ptr1 = allocator.allocate(1);
        REQUIRE(ptr1 - ptr0 == 1);
    }

    {
        using traits = std::allocator_traits<kumar::RegionAllocator<int>>;
        int* ptr0 = traits::allocate(allocator, 1);
        int* ptr1 = traits::allocate(allocator, 1);
        REQUIRE(ptr1 - ptr0 == 1);
    }

    {
        std::vector<int, kumar::RegionAllocator<int>> v0(allocator);
        int* ptr0 = allocator.allocate(1);
        *ptr0 = 0xdeadbeef;
        v0.reserve(100);
        int* ptr1 = allocator.allocate(1);
        *ptr1 = 0xfee1dead;
        for (std::size_t i = 0; i < v0.capacity(); ++i)
        {
            v0.emplace_back(static_cast<int>(i));
        }
        REQUIRE(*ptr0 == 0xdeadbeef);
        REQUIRE(*ptr1 == 0xfee1dead);
        std::vector<int, kumar::RegionAllocator<int>> v1(v0, allocator);
        std::vector<int, kumar::RegionAllocator<int>> v2(std::move(v0), allocator);
    }
}
