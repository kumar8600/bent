#pragma once

#include <cstdint>
#include <vector>
#include <stack>
#include <bitset>
#include <utility>
#include <memory>

#include "definitions.hpp"
#include "component_pool.hpp"
#include "../component_manager.hpp"

namespace bent
{
    struct View;
    struct World;

    struct EntityManager
    {
        using ComponentMask = std::bitset<MAX_COMPONENTS>;

        std::pair<std::uint32_t, std::uint32_t> CreateEntity()
        {
            if (free_list_.empty())
            {
                std::uint32_t index = entity_versions_.size();
                entity_alive_flags_.emplace_back(true);
                entity_versions_.emplace_back(0);
                entity_component_masks_.emplace_back();
                return std::pair<std::uint32_t, std::uint32_t>(index, 0);
            }
            else
            {
                auto index = free_list_.top(); free_list_.pop();
                auto version = entity_versions_[index]; // version is incremented at DestroyEntity
                assert(entity_alive_flags_[index] == false);
                entity_alive_flags_[index] = true;
                return std::pair<std::uint32_t, std::uint32_t>(index, version);
            }
        }

        void DestroyEntity(std::uint32_t index)
        {
            auto && aliver = entity_alive_flags_[index];
            if (!aliver)
            {
                throw std::out_of_range("This entity has already have dead");
            }
            auto & mask = entity_component_masks_[index];
            for (std::uint16_t i = 0; i < MAX_COMPONENTS; i++)
            {
                if (mask[i])
                {
                    RemoveComponent(index, i);
                }
            }
            aliver = false;
            ++entity_versions_[index];
            free_list_.push(index);
        }

        bool alive(std::uint32_t index) const
        {
            return entity_alive_flags_[index];
        }

        std::uint32_t version(std::uint32_t index) const
        {
            return entity_versions_[index];
        }

        ComponentMask component_mask(std::uint32_t index) const
        {
            return entity_component_masks_[index];
        }

        template <typename T, typename... Args>
        void AddComponent(std::uint32_t index, std::uint16_t component_index, Args&&... args)
        {
            auto & mask = entity_component_masks_[index];
            if (mask[component_index])
            {
                throw std::out_of_range("This entity has already have this component");
            }
            auto & pool = component_pool(component_index);
            auto p = pool.Allocate(index);
            new (p) T(std::forward<Args>(args)...);
            mask[component_index] = true;
        }

        void AddComponentFrom(std::uint32_t index, std::uint16_t component_index, const void * src)
        {
            auto & mask = entity_component_masks_[index];
            if (mask[component_index])
            {
                throw std::out_of_range("This entity has already have this component");
            }
            auto & pool = component_pool(component_index);
            auto p = pool.Allocate(index);
            ComponentManager::instance().dynamic_constructor(component_index).CopyConstruct(p, src);
            mask[component_index] = true;
        }

        void AddComponentFromMove(std::uint32_t index, std::uint16_t component_index, void * src)
        {
            auto & mask = entity_component_masks_[index];
            if (mask[component_index])
            {
                throw std::out_of_range("This entity has already have this component");
            }
            auto & pool = component_pool(component_index);
            auto p = pool.Allocate(index);
            ComponentManager::instance().dynamic_constructor(component_index).MoveConstruct(p, src);
            mask[component_index] = true;
        }

        void * GetComponent(std::uint32_t index, std::uint16_t component_index)
        {
            auto & mask = entity_component_masks_[index];
            if (!mask[component_index])
            {
                return nullptr;
            }
            auto & pool = component_pool(component_index);
            return pool.Get(index);
        }

        void RemoveComponent(std::uint32_t index, std::uint16_t component_index)
        {
            auto p = GetComponent(index, component_index);
            if (p == nullptr)
            {
                throw std::out_of_range("This entity does not have this component");
            }
            ComponentManager::instance().dynamic_constructor(component_index).Destroy(p);
            entity_component_masks_[index][component_index] = false;
        }

    private:
        friend View;
        friend World;

        using EntityVersionVector = std::vector<std::uint32_t>;
        using EntityAliveFlagVector = std::vector<bool>;
        using ComponentMaskVector = std::vector<ComponentMask>;
        using ComponentPoolPtrVector = std::vector<std::unique_ptr<ComponentPoolInterface>>;
        using FreeListStack = std::stack<std::uint32_t>;

        EntityManager() :
            component_pools_(MAX_COMPONENTS)
        {
        }

        ComponentPoolInterface & component_pool(std::uint16_t component_index)
        {
            auto& poolp = component_pools_[component_index];
            if (!poolp)
            {
                poolp.reset(ComponentManager::instance().component_pool_factory(component_index).Create());
            }
            return *poolp;
        }

        EntityAliveFlagVector entity_alive_flags_;
        EntityVersionVector entity_versions_;
        ComponentMaskVector entity_component_masks_;
        ComponentPoolPtrVector component_pools_;

        FreeListStack free_list_;
    };
}
