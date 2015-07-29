#include "bent/internal/region.hpp"

#include <memory>

/// Constructs a region.
///
/// @param initial_size  Default size of buffer. In default, this is 64 MiB.

kumar::Region::Region(size_type initial_reserve)
{
    std::unique_ptr<char []> buffer(new char[initial_reserve]);
    back_ = buffer.get();
    size_remain_ = initial_reserve;
    allocated_ = 0;
    auto end = buffer.get() + initial_reserve;
    buffers_.emplace_back(std::move(buffer), end);
}

/// Move constructs a region.

kumar::Region::Region(Region && other)
{
    operator=(std::forward<Region>(other));
}

/// Move assignments a region.

kumar::Region& kumar::Region::operator=(Region&& other)
{
    std::lock_guard<std::shared_timed_mutex> lock(mtx_);
    std::shared_lock<std::shared_timed_mutex> lock_other(other.mtx_);

    buffers_ = std::move(other.buffers_);
    back_ = other.back_;
    size_remain_ = other.size_remain_;
    allocated_ = other.allocated_;

    return *this;
}

/// Allocates a memory.
///
/// If buffer is used up, new buffer is added.
/// @param alignment    The alignment of object to allocate storage for
/// @param size         The size of object to allocate storage for
/// @oaran num          The number of objects to allocate storage for
/// @return allocated memory

void * kumar::Region::Allocate(size_type alignment, size_type size, size_type num)
{
    std::lock_guard<std::shared_timed_mutex> lock(mtx_);
    size_type size_to_allocate = std::max(size, alignment) * num;
    {
        void* back = back_;
        size_type size_remain = size_remain_;
        auto ptr = std::align(alignment, size_to_allocate, back, size_remain);
        if (ptr)
        {
            back_ = (char*) back + size_to_allocate;
            size_remain_ = size_remain - size_to_allocate;
            allocated_ += size_to_allocate;
            return ptr;
        }
    }

    // if there are no usable storages, creates a new storage
    {
        auto buffer_size = std::max(size_to_allocate + alignment - 1, allocated_ * 2);
        std::unique_ptr<char []> buffer(new char[buffer_size]);
        void* back = buffer.get();
        buffers_.emplace_back(std::move(buffer), (char*) back + buffer_size);
        auto size_remain = buffer_size;
        auto ptr = std::align(alignment, size_to_allocate, back, size_remain);
        if (ptr)
        {
            back_ = (char*) back + size_to_allocate;
            size_remain_ = size_remain - size_to_allocate;
            allocated_ += size_to_allocate;
        }
        return ptr;
    }
}

/// Returns a number of buffers.

kumar::Region::size_type kumar::Region::buffer_count() const
{
    std::shared_lock<std::shared_timed_mutex> lock(mtx_);
    return buffers_.size();
}

/// Returns a sum of the allocated memories.

kumar::Region::size_type kumar::Region::size_allocated() const
{
    std::shared_lock<std::shared_timed_mutex> lock(mtx_);
    return allocated_;
}

/// Returns a reserved memory size.
///
/// Even if used size is smaller than this, new buffer adding is happen.

kumar::Region::size_type kumar::Region::size_reserved() const
{
    std::shared_lock<std::shared_timed_mutex> lock(mtx_);
    size_type sum = 0;
    for (auto& buffer : buffers_)
    {
        sum += buffer.second - buffer.first.get();
    }
    return sum;
}

