#pragma once

#include <bitset>

#include "internal/definitions.hpp"
#include "internal/entity_id_manager.hpp"
#include "internal/world_state.hpp"
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
        EntityHandle Create();

        /// Gets an entity.
        ///
        /// @return entity handle that found.
        /// When not found, throws out_of_range exception.
        EntityHandle Get(entity_id_t entity_id);

        /// Runs Garbage Collection.
        ///
        /// Sweeps removed entities and components. (And some optimizations)
        void CollectGarbage();

        /// Returns a view with entities that have components requried.
        template <typename... Args>
        View entities_with()
        {
            std::bitset<MAX_COMPONENTS> component_mask;
            for (auto& i : std::initializer_list<component_type_id_t> { ComponentManager::instance().id<Args>()... })
            {
                component_mask[i] = true;
            }

            return entities_with(component_mask);
        }

        /// Returns a view with entities that have components requried by names.
        View entities_with(int argc,  const char * argv[]);

    private:

        /// Returns a view with entities that have components requried by bit mask.
        View entities_with(const std::bitset<MAX_COMPONENTS> & component_mask);

        EntityIdManager entity_id_manager_;
        WorldState state_;
    };
}
