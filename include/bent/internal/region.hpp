#pragma once

#include <cstddef>
#include <deque>
#include <memory>
#include <shared_mutex>
#include <algorithm>

namespace kumar
{
    /// An class for region-based memory management.
    ///
    /// An operations for this class is thread-safe.
    /// If buffer is used up, new buffer is added.
    struct Region
    {
        using size_type = std::size_t;

        /// Constructs a region.
        ///
        /// @param initial_size  Default size of buffer. In default, this is 64 MiB.
        explicit Region(size_type initial_reserve = 67108864 /* 64 MiB */);

        Region(const Region&) = delete;
        Region& operator=(const Region& other) = delete;

        /// Move constructs a region.
        Region(Region&& other);

        /// Move assignments a region.
        Region& operator=(Region&& other);

        /// Allocates a memory.
        ///
        /// If buffer is used up, new buffer is added.
        /// @param alignment    The alignment of object to allocate storage for
        /// @param size         The size of object to allocate storage for
        /// @oaran num          The number of objects to allocate storage for
        /// @return allocated memory
        void* Allocate(size_type alignment, size_type size, size_type num);

        /// Returns a number of buffers.
        size_type buffer_count() const;

        /// Returns a sum of the allocated memories.
        size_type size_allocated() const;

        /// Returns a reserved memory size.
        ///
        /// Even if used size is smaller than this, new buffer adding is happen.
        size_type size_reserved() const;

    private:
        std::deque<std::pair<std::unique_ptr<char []>, char*>> buffers_;
        char* back_;
        size_type size_remain_;
        size_type allocated_ = 0;
        mutable std::shared_timed_mutex mtx_;
    };
}
