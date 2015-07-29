#pragma once

#include <bitset>
#include <deque>

#include "definitions.hpp"
#include "region.hpp"
#include "region_allocator.hpp"

namespace bent
{
    struct View;
    struct EntityHandle;

    struct WorldState
    {
        struct TemporalEntityHandle;

        // constructors

        WorldState();

        WorldState(const WorldState& other);

        // destructor

        ~WorldState();

        // getters

        state_version_t version() const;

        // entity access

        TemporalEntityHandle AddEntity(entity_id_t entity_id);

        void RemoveEntity(TemporalEntityHandle pos);

        // component access

        template <typename T, typename... Args>
        void AddComponent(TemporalEntityHandle pos, component_type_id_t component_id, Args&&... args)
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
            kumar::RegionAllocator<T> alloc(region_);
            auto p = alloc.allocate(1);
            new (p) T(std::forward<Args>(args)...);
            InsertComponentPointer(entity, component_id, p);
        }

        void AddComponentFrom(TemporalEntityHandle pos, component_type_id_t component_id, const void* val);

        void AddComponentFromMove(TemporalEntityHandle pos, component_type_id_t component_id, void* val);

        void RemoveComponent(TemporalEntityHandle pos, component_type_id_t component_id);

        void * GetComponent(TemporalEntityHandle pos, component_type_id_t component_id);

        // entity lookup

        bool IsEntityValid(TemporalEntityHandle temporal_handle) const;

        TemporalEntityHandle FindEntity(entity_id_t entity_id);

        // garbage collection

        void CollectGarbage();

        /// Temporal entity handle.
        ///
        /// This is invalidated when collect_garbage is called.
        struct TemporalEntityHandle
        {
            bool operator==(const TemporalEntityHandle& rhs) const
            {
                return entity_ptr_ == rhs.entity_ptr_;
            }

            bool operator!=(const TemporalEntityHandle& rhs) const
            {
                return !operator==(rhs);
            }

            explicit operator bool() const
            {
                return entity_ptr_ != nullptr;
            }

        private:
            friend WorldState;
            friend EntityHandle;
            friend View;

            TemporalEntityHandle(void * entity_ptr) :
                entity_ptr_(entity_ptr)
            {}

            void * entity_ptr_;
        };

    private:
        // internal types
        
        friend View;

        struct component_ptrs_t
        {
            component_ptrs_t() :
                ptrs(nullptr), size(0), reserved(0)
            {}

            void** ptrs;
            component_type_id_t size;
            component_type_id_t reserved;
        };

        struct entity_t
        {
            entity_t(entity_id_t id) :
                id(id), marked_as_garbage(false)
            {}

            entity_id_t id;
            std::bitset<MAX_COMPONENTS> component_mask;
            component_ptrs_t component_ptrs;
            bool marked_as_garbage;
        };

        using EntityContainer = std::deque<entity_t, kumar::RegionAllocator<entity_t>>;

        // utilities

        void InsertComponentPointer(component_ptrs_t& ptrs, component_type_id_t index, void* ptr);

        void EraseComponentPointer(component_ptrs_t& ptrs, component_type_id_t index);

        void ** GetComponentPointerPointer(const entity_t& entity, component_type_id_t component_id);

        void InsertComponentPointer(entity_t& entity, component_type_id_t component_id, void * ptr);

        void EraseComponentPointer(entity_t& entity, component_type_id_t component_id);

        void * GetComponentPointer(const entity_t& entity, component_type_id_t component_id);

        entity_t& ToEntity(TemporalEntityHandle temporal_handle) const;


        void CopyAllEntitiesButNoComponentValues(const EntityContainer & from, EntityContainer & to, kumar::Region & region);

        void CopyAllComponentValues(EntityContainer & entities, kumar::Region & region);

        void MoveAllComponentValuesAndDestroy(EntityContainer & entities, kumar::Region & region);


        // members

        kumar::Region region_;
        EntityContainer entities_;
        state_version_t version_ = 0;
    };
}
