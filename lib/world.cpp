#include "bent/world.hpp"

#include <bitset>
#include <stdexcept>

/// Creates an entity.
///
/// @return entity handle refering created entity.

bent::EntityHandle bent::World::Create()
{
    auto id = entity_id_manager_.Create();
    auto th = state_.AddEntity(id);
    return EntityHandle(id, state_, th);
}

/// Gets an entity.
///
/// @return entity handle that found.
/// When not found, throws out_of_range exception.

bent::EntityHandle bent::World::Get(entity_id_t entity_id)
{
    auto th = state_.FindEntity(entity_id);
    if (!th)
    {
        throw std::out_of_range("Entity not found");
    }
    return EntityHandle(entity_id, state_, th);
}

/// Runs Garbage Collection.
///
/// Sweeps removed entities and components. (And some optimizations)

void bent::World::CollectGarbage()
{
    state_.CollectGarbage();
}

/// Returns a view with entities that have components requried by names.
bent::View bent::World::entities_with(int argc, const char * argv[])
{
    std::bitset<MAX_COMPONENTS> component_mask;
    for (auto i = 0; i < argc; ++i)
    {
        component_mask[ComponentManager::instance().id(argv[i])] = true;
    }

    return entities_with(component_mask);
}

/// Returns a view with entities that have components requried by bit mask.

bent::View bent::World::entities_with(const std::bitset<MAX_COMPONENTS> & component_mask)
{
    return View(state_, component_mask);
}
