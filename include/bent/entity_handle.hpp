#pragma once

#include <string>
#include <type_traits>

#include "component_manager.hpp"
#include "internal/definitions.hpp"
#include "internal/world_state.hpp"

namespace bent
{
    struct World;
    struct View;

    struct EntityHandle
    {
        // getters

        /// Returns an id of the entity this handle refering.
        entity_id_t id() const
        {
            return id_;
        }

        // entity access

        /// Destroys this entity.
        ///
        /// This handle is invalidated.
        void Destroy()
        {
            RefreshCache();
            world_state_->RemoveEntity(cache_.temporal_handle);
        }

        /// Returns whether this entity handle is valid or not.
        bool valid() const
        {
            if (cache_.version == world_state_->version())
            {
                return world_state_->IsEntityValid(cache_.temporal_handle);
            }
            else
            {
                auto th = world_state_->FindEntity(id());
                return bool(th);
            }
        }

        // component access

        /// Adds a component T for this entity.
        template <typename T, typename... Args>
        void Add(Args&&... args)
        {
            RefreshCache();
            world_state_->AddComponent<T>(cache_.temporal_handle, ComponentManager::instance().id<T>(), std::forward<Args>(args)...);
        }

        /// Adds a component for this entity by copying.
        template <typename T>
        void AddFrom(T && value)
        {
            using ValT = typename std::remove_reference<T>::type;
            RefreshCache();
            world_state_->AddComponent<ValT>(cache_.temporal_handle, ComponentManager::instance().id<ValT>(), std::forward<T>(value));
        }

        /// Removes a component T attached for this entity.
        template <typename T>
        void Remove()
        {
            RefreshCache();
            world_state_->RemoveComponent(cache_.temporal_handle, ComponentManager::instance().id<T>());
        }

        /// Returns a pointer to a component T attached to this entity.
        ///
        /// If component is not exists, returns nullptr.
        /// This pointer is invalidated when `bent::world::collect_garbage` is called.
        template <typename T>
        T* Get()
        {
            RefreshCache();
            return static_cast<T*>(world_state_->GetComponent(cache_.temporal_handle, ComponentManager::instance().id<T>()));
        }

        // component access without type

        /// Adds a component for this entity by copying.
        void AddFrom(const std::string& component_name, const void * value)
        {
            RefreshCache();
            world_state_->AddComponentFrom(cache_.temporal_handle, ComponentManager::instance().id(component_name), value);
        }

        /// Adds a component for this entity by moving.
        void AddFromMove(const std::string& component_name, void * value)
        {
            RefreshCache();
            world_state_->AddComponentFromMove(cache_.temporal_handle, ComponentManager::instance().id(component_name), value);
        }

        /// Removes a component attached for for this entity.
        void Remove(const std::string& component_name)
        {
            RefreshCache();
            world_state_->RemoveComponent(cache_.temporal_handle, ComponentManager::instance().id(component_name));
        }

        /// Returns a pointer to a component attached to this entity.
        ///
        /// If component is not exists, returns nullptr.
        /// This pointer is invalidated when `bent::world::collect_garbage` is called.
        void* Get(const std::string& component_name)
        {
            RefreshCache();
            return world_state_->GetComponent(cache_.temporal_handle, ComponentManager::instance().id(component_name));
        }

        // operators

        bool operator==(const EntityHandle& rhs) const
        {
            return world_state_ == rhs.world_state_ && id_ == rhs.id_;
        }

        bool operator!=(const EntityHandle& rhs) const
        {
            return !operator==(rhs);
        }

    private:

        void RefreshCache() const
        {
            if (cache_.version == world_state_->version())
            {
                return;
            }
            else
            {
                auto th = world_state_->FindEntity(id());
                if (!th)
                {
                    throw std::logic_error("Invalid entity handle is used");
                }
                cache_.temporal_handle = th;
                cache_.version = world_state_->version();
            }
        }

        // constructors

        friend World;
        friend View;

        EntityHandle(WorldState & world_state) :
            id_(INVALID_ENTITY_ID),
            world_state_(&world_state),
            cache_(INVALID_STATE_VERSION, WorldState::TemporalEntityHandle(nullptr))
        {}

        EntityHandle(entity_id_t id, WorldState & world_state, WorldState::TemporalEntityHandle temporal_handle) :
            id_(id),
            world_state_(&world_state),
            cache_(world_state.version(), temporal_handle)
        {}

        struct cache_t
        {
            cache_t(state_version_t version, WorldState::TemporalEntityHandle temporal_handle) :
                version(version),
                temporal_handle(temporal_handle)
            {}

            state_version_t version;
            WorldState::TemporalEntityHandle temporal_handle;
        };

        entity_id_t id_;
        WorldState * world_state_;
        mutable cache_t cache_;
    };
}
