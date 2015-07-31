#pragma once

#include <bitset>
#include <string>

#include "internal/definitions.hpp"
#include "internal/entity_manager.hpp"
#include "entity_handle.hpp"
#include "view.hpp"

namespace bent
{
    struct EntityHandle;
    struct View;

    struct World
    {
        /// Creates an entity.
        ///
        /// @return entity handle refering created entity.
        EntityHandle Create()
        {
            auto res = entity_manager_.CreateEntity();
            return EntityHandle(entity_manager_, res.first, res.second);
        }

        /// Gets an entity handle.
        ///
        /// @return entity handle that found.
        /// When not found, throws out_of_range exception.
        EntityHandle entity(std::uint64_t entity_id)
        {
            std::uint32_t index = static_cast<std::uint32_t>(entity_id);
            std::uint32_t version = static_cast<std::uint32_t>(entity_id >> 32UL);
            if (entity_manager_.alive(index) && entity_manager_.version(index) == version)
            {
                return EntityHandle(entity_manager_, index, version);
            }
            else
            {
                throw std::out_of_range("Entity " + std::to_string(entity_id) + " not found");
            }
        }

        /// Returns a view with entities that have components requried.
        template <typename... Args>
        View entities_with()
        {
            ComponentMask component_mask;
            for (auto& i : std::initializer_list<std::uint16_t> { ComponentManager::instance().id<Args>()... })
            {
                component_mask[i] = true;
            }

            return entities_with(component_mask);
        }

        /// Returns a view with entities that have components requried by names.
        View entities_with(int argc, const char * argv [])
        {
            ComponentMask component_mask;
            for (auto i = 0; i < argc; ++i)
            {
                component_mask[ComponentManager::instance().id(argv[i])] = true;
            }

            return entities_with(component_mask);
        }

    private:

        using ComponentMask = EntityManager::ComponentMask;

        /// Returns a view with entities that have components requried by bit mask.
        View entities_with(const ComponentMask & component_mask)
        {
            return View(entity_manager_, component_mask);
        }

        EntityManager entity_manager_;
    };
}
