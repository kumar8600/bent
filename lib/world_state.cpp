#include "bent/internal/world_state.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include "bent/component_manager.hpp"

bent::WorldState::WorldState() :
    region_(65536u /* 64 KiB */),
    entities_(kumar::RegionAllocator<entity_t>(region_)),
    version_(0)
{}

bent::WorldState::WorldState(const WorldState & other) :
    region_(std::max(other.region_.size_allocated(), std::size_t(65536U) /* 64 KiB */)),
    entities_(kumar::RegionAllocator<entity_t>(region_)),
    version_(other.version_ + 1)
{
    CopyAllEntitiesButNoComponentValues(other.entities_, entities_, region_);
    CopyAllComponentValues(entities_, region_);
}

bent::WorldState::~WorldState()
{
    // call all component's destructors
    for (component_type_id_t i = 0; i < ComponentManager::instance().size(); ++i)
    {
        auto& alloc = ComponentManager::instance().allocator(i);
        for (auto& entity : entities_)
        {
            if (entity.component_mask[i])
            {
                auto p = GetComponentPointer(entity, i);
                alloc.Destroy(p);
            }
        }
    }
}

bent::state_version_t bent::WorldState::version() const
{
    return version_;
}

bent::WorldState::TemporalEntityHandle bent::WorldState::AddEntity(entity_id_t entity_id)
{
    if (!entities_.empty() && entity_id <= entities_.back().id)
    {
        throw std::out_of_range("entity id must be greater than before");
    }
    entities_.emplace_back(entity_id);
    return TemporalEntityHandle(&entities_.back());
}

void bent::WorldState::RemoveEntity(TemporalEntityHandle pos)
{
    auto& mark = ToEntity(pos).marked_as_garbage;
    if (mark)
    {
        throw std::out_of_range("this entity has already be removed");
    }
    mark = true;
}

void bent::WorldState::AddComponentFrom(TemporalEntityHandle pos, component_type_id_t component_id, const void * val)
{
    auto& entity = ToEntity(pos);
    if (entity.marked_as_garbage)
    {
        throw std::out_of_range("this entity has already been removed");
    }
    if (entity.component_mask[component_id])
    {
        throw std::out_of_range("this entity has already had this component");
    }
    auto& alloc = ComponentManager::instance().allocator(component_id);
    auto p = alloc.Allocate(1, region_);
    alloc.CopyConstruct(p, val);
    InsertComponentPointer(entity, component_id, p);
}

void bent::WorldState::AddComponentFromMove(TemporalEntityHandle pos, component_type_id_t component_id, void * val)
{
    auto& entity = ToEntity(pos);
    if (entity.marked_as_garbage)
    {
        throw std::out_of_range("this entity has already been removed");
    }
    if (entity.component_mask[component_id])
    {
        throw std::out_of_range("this entity has already had this component");
    }
    auto& alloc = ComponentManager::instance().allocator(component_id);
    auto p = alloc.Allocate(1, region_);
    alloc.MoveConstruct(p, val);
    InsertComponentPointer(entity, component_id, p);
}

void bent::WorldState::RemoveComponent(TemporalEntityHandle pos, component_type_id_t component_id)
{
    auto& entity = ToEntity(pos);
    if (entity.marked_as_garbage)
    {
        throw std::out_of_range("this entity has already been removed");
    }
    auto&& mask = entity.component_mask[component_id];
    if (!mask)
    {
        throw std::out_of_range("this entity does not have this component");
    }
    auto& alloc = ComponentManager::instance().allocator(component_id);
    auto p = GetComponentPointer(entity, component_id);
    EraseComponentPointer(entity, component_id);
    alloc.Destroy(p);
}

void * bent::WorldState::GetComponent(TemporalEntityHandle pos, component_type_id_t component_id)
{
    auto& entity = ToEntity(pos);
    if (entity.marked_as_garbage)
    {
        throw std::out_of_range("this entity has already been removed");
    }
    auto&& mask = entity.component_mask[component_id];
    if (!mask)
    {
        return nullptr;
    }
    auto p = GetComponentPointer(entity, component_id);
    return p;
}

bool bent::WorldState::IsEntityValid(TemporalEntityHandle temporal_handle) const
{
    return !ToEntity(temporal_handle).marked_as_garbage;
}

bent::WorldState::TemporalEntityHandle bent::WorldState::FindEntity(entity_id_t entity_id)
{
    auto it = std::lower_bound(entities_.begin(), entities_.end(), entity_id, [](const entity_t & lhs, entity_id_t rhs)
    {
        return lhs.id < rhs;
    });
    if (it != entities_.end() && it->id == entity_id && !it->marked_as_garbage)
    {
        return TemporalEntityHandle(&*it);
    }
    else
    {
        return TemporalEntityHandle(nullptr);
    }
}

void bent::WorldState::CollectGarbage()
{
    kumar::Region region(region_.size_allocated());
    std::swap(region, region_);
    decltype(entities_) entities((kumar::RegionAllocator<entity_t>(region_)));

    CopyAllEntitiesButNoComponentValues(entities_, entities, region_);
    MoveAllComponentValuesAndDestroy(entities, region_);

    entities_ = std::move(entities);
    ++version_;
}

void bent::WorldState::InsertComponentPointer(component_ptrs_t & ptrs, component_type_id_t index, void * ptr)
{
    if (ptrs.size != ptrs.reserved)
    {
        // insert
#ifdef _MSC_VER
        stdext::checked_array_iterator<void**> p(ptrs.ptrs, ptrs.size + 1);
#else
        auto p = ptrs.ptrs;
#endif
        auto end = p + ptrs.size;

        std::copy_backward(p + index, end, end + 1);
        ptrs.ptrs[index] = ptr;
        ++(ptrs.size);
    }
    else
    {
        // reserve and insert
        auto newreserved = ptrs.reserved == 0 ? 8 : ptrs.reserved * 2;
        kumar::RegionAllocator<void*> alloc(region_);
        auto allocated = alloc.allocate(newreserved);
#ifdef _MSC_VER
        stdext::checked_array_iterator<void**> p(allocated, newreserved);
#else
        auto p = allocated;
#endif
        std::copy(ptrs.ptrs, ptrs.ptrs + index, p);
        std::copy(ptrs.ptrs + index, ptrs.ptrs + ptrs.size, p + index + 1);
        p[index] = ptr;

#ifdef _MSC_VER
        ptrs.ptrs = p.base();
#else
        ptrs.ptrs = p;
#endif
        ptrs.reserved = newreserved;
        ++(ptrs.size);
    }
}

void bent::WorldState::EraseComponentPointer(component_ptrs_t & ptrs, component_type_id_t index)
{
#ifdef _MSC_VER
    stdext::checked_array_iterator<void**> p(ptrs.ptrs, ptrs.size);
#else
    auto p = ptrs.ptrs;
#endif
    if (index != 0)
    {
        std::copy(p + index, p + ptrs.size, p + index - 1);
    }
    --(ptrs.size);
}

void ** bent::WorldState::GetComponentPointerPointer(const entity_t & entity, component_type_id_t component_id)
{
    auto mask = entity.component_mask << (entity.component_mask.size() - component_id);
    auto index = mask.count();
    return entity.component_ptrs.ptrs + index;
}

void bent::WorldState::InsertComponentPointer(entity_t & entity, component_type_id_t component_id, void * ptr)
{
    InsertComponentPointer(entity.component_ptrs, static_cast<component_type_id_t>(GetComponentPointerPointer(entity, component_id) - entity.component_ptrs.ptrs), ptr);
    entity.component_mask[component_id] = true;
}

void bent::WorldState::EraseComponentPointer(entity_t & entity, component_type_id_t component_id)
{
    EraseComponentPointer(entity.component_ptrs, static_cast<component_type_id_t>(GetComponentPointerPointer(entity, component_id) - entity.component_ptrs.ptrs));
    entity.component_mask[component_id] = false;
}

void * bent::WorldState::GetComponentPointer(const entity_t & entity, component_type_id_t component_id)
{
    return *GetComponentPointerPointer(entity, component_id);
}

bent::WorldState::entity_t & bent::WorldState::ToEntity(TemporalEntityHandle temporal_handle) const
{
    return *static_cast<entity_t*>(temporal_handle.entity_ptr_);
}

void bent::WorldState::CopyAllEntitiesButNoComponentValues(const EntityContainer & from, EntityContainer & to, kumar::Region & region)
{
    for (auto& e : from)
    {
        if (!e.marked_as_garbage)
        {
            to.emplace_back(e);
        }
    }

    for (auto& e : to)
    {
        auto old_ptrs = e.component_ptrs.ptrs;
        e.component_ptrs.reserved = e.component_ptrs.size;
        e.component_ptrs.ptrs = kumar::RegionAllocator<void*>(region).allocate(e.component_ptrs.reserved);
#ifdef _MSC_VER
        std::copy(old_ptrs, old_ptrs + e.component_ptrs.size, stdext::checked_array_iterator<void**>(e.component_ptrs.ptrs, e.component_ptrs.size));
#else
        std::copy(old_ptrs, old_ptrs + e.component_ptrs.size, e.component_ptrs.ptrs);
#endif
    }
}

void bent::WorldState::CopyAllComponentValues(EntityContainer & entities, kumar::Region & region)
{
    for (component_type_id_t i = 0; i < ComponentManager::instance().size(); ++i)
    {
        auto& allocator = ComponentManager::instance().allocator(i);
        for (auto& e : entities)
        {
            if (e.component_mask[i])
            {
                auto p = allocator.Allocate(1, region);
                auto src = GetComponentPointerPointer(e, i);
                allocator.CopyConstruct(p, *src);
                *src = p;
            }
        }
    }
}

void bent::WorldState::MoveAllComponentValuesAndDestroy(EntityContainer & entities, kumar::Region & region)
{
    for (component_type_id_t i = 0; i < ComponentManager::instance().size(); ++i)
    {
        auto& allocator = ComponentManager::instance().allocator(i);
        for (auto& e : entities)
        {
            if (e.component_mask[i])
            {
                auto p = allocator.Allocate(1, region);
                auto src = GetComponentPointerPointer(e, i);
                allocator.MoveConstruct(p, *src);
                allocator.Destroy(*src);
                *src = p;
            }
        }
    }
}

