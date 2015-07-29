#pragma once

#include <cstddef>
#include <memory>

#include "region.hpp"
#include "region_allocator.hpp"

namespace kumar
{
    struct DynamicRegionAllocatorInterface
    {
        virtual ~DynamicRegionAllocatorInterface() = default;
        virtual void* Allocate(std::size_t n, kumar::Region & region) = 0;
        virtual void Deallocate(void* p, std::size_t n, kumar::Region & region) = 0;
        virtual void CopyConstruct(void* p, const void* src) = 0;
        virtual void MoveConstruct(void* p, void* src) = 0;
        virtual void Destroy(void* p) = 0;
    };

    template <typename T>
    struct DynamicRegionAllocator : DynamicRegionAllocatorInterface
    {
        virtual void* Allocate(std::size_t n, kumar::Region & region) override
        {
            kumar::RegionAllocator<T> alloc(region);
            return alloc.allocate(n);
        }

        virtual void Deallocate(void * p, std::size_t n, kumar::Region & region) override
        {
            kumar::RegionAllocator<T> alloc(region);
            alloc.deallocate(static_cast<T*>(p), n);
        }

        virtual void CopyConstruct(void* p, const void* src) override
        {
            const T& ref = *static_cast<const T*>(src);
            new (p) T(ref);
        }

        virtual void MoveConstruct(void* p, void* src) override
        {
            T& ref = *static_cast<T*>(src);
            new (p) T(std::move(ref));
        }

        virtual void Destroy(void* p) override
        {
            static_cast<T*>(p)->~T();
        }
    };
}
