#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <cassert>

namespace bent
{
    struct ComponentPoolInterface
    {
        virtual ~ComponentPoolInterface() = default;

        virtual void * Allocate(std::uint32_t index) = 0;
        virtual void * Get(std::uint32_t index) = 0;
    };

    template <typename T>
    struct ComponentPool : ComponentPoolInterface
    {
        explicit ComponentPool(std::size_t chunk_size = 8192) :
            block_size_(chunk_size / sizeof(T))
        {}

        /// Allocates a memory for a component of the entity indexed INDEX.
        ///
        /// Notice: You must construct and destruct allocated memory on your responsibility.
        virtual void * Allocate(std::uint32_t index) override
        {
            auto i = index / block_size_;
            auto j = index % block_size_;
            if (blocks_.size() <= i)
            {
                blocks_.resize(i + 1);
            }
            auto & block = blocks_[i];
            if (!block)
            {
                block.reset(new Element[block_size_]);
            }
            return std::addressof(block[j]);
        }

        /// Returns a pointer refering the component of the entity indexed INDEX.
        virtual void * Get(std::uint32_t index) override
        {
            return std::addressof(GetRef(index));
        }

    private:

        T& GetRef(std::uint32_t index)
        {
            auto i = index / block_size_;
            auto j = index % block_size_;
            assert(i < blocks_.size());
            auto & block = blocks_[i];
            assert(block);
            return *reinterpret_cast<T*>(std::addressof(block[j]));
        }

        using Element = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
        using BlockContainer = std::vector<std::unique_ptr<Element []>>;

        BlockContainer blocks_;
        std::size_t block_size_;
    };
}
