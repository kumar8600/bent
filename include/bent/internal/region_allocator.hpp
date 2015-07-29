#pragma once

#include <cstddef>
#include <type_traits>

#include "region.hpp"

namespace kumar
{
    /// An allocator for region-based memory management
    ///
    /// You must pass a region reference to construct this class.
    /// If you want to use this class directly, use via std::allocator_traits.
    template <typename T>
    struct RegionAllocator
    {
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef std::size_t size_type;
        typedef std::true_type propagate_on_container_move_assignment;

        template <typename U>
        struct rebind
        {
            typedef RegionAllocator<U> other;
        };

        /// Constructs a region allocator.
        ///
        /// @param region   A region used for allocation.
        explicit RegionAllocator(Region & region) :
            region_(&region)
        {
        }

        template <class U>
        RegionAllocator(const RegionAllocator<U> & other) noexcept :
        region_(&other.region())
        {
        }

        /// Allocates a memory for n objects.
        pointer allocate(size_type n)
        {
            return static_cast<value_type*>(region_->Allocate(alignof(value_type), sizeof(value_type), n));
        }

        /// Do nothing.
        void deallocate(value_type* p, size_type n)
        {
        }

        /// Returns a referrence of region used for allocation.
        Region& region() const
        {
            return *region_;
        }

    private:
        Region* region_;
    };

    template <class T, class U>
    bool operator==(const RegionAllocator<T>& lhs, const RegionAllocator<U>& rhs) noexcept
    {
        return &lhs.region() == &rhs.region();
    }

    template <class T, class U>
    bool operator!=(const RegionAllocator<T>& lhs, const RegionAllocator<U>& rhs) noexcept
    {
        return !operator==(lhs, rhs);
    }
}
