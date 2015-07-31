#pragma once

#include "component_pool.hpp"

namespace bent
{
    struct ComponentPoolFactoryInterface
    {
        virtual ~ComponentPoolFactoryInterface() = default;

        virtual ComponentPoolInterface * Create(std::size_t chunk_size = 8192) = 0;
    };

    template<typename T>
    struct ComponentPoolFactory : ComponentPoolFactoryInterface
    {
        virtual ComponentPoolInterface * Create(std::size_t chunk_size = 8192)
        {
            return new ComponentPool<T>(chunk_size);
        }
    };
}
