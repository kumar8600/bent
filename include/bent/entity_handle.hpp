#pragma once

#include <stdexcept>
#include <string>

#include "internal/entity_manager.hpp"
#include "component_manager.hpp"

namespace bent
{
    struct World;
    struct View;

    struct EntityHandle
    {
        // getters

        /// Returns an id of this entity.
        ///
        /// This is unique in an local execution
        std::uint64_t id() const
        {
            return std::uint64_t(index_) | std::uint64_t(version_) << 32UL;
        }

        /// Returns whether this entity is valid or not.
        bool valid() const
        {
            return entity_manager_->version(index_) == version_;
        }

        // entity access

        /// Destroys this entity.
        ///
        /// All components attached to this entity are destroyed too.
        void Destroy()
        {
            ThrowsIfInvalid();
            entity_manager_->DestroyEntity(index_);
        }

        // component access

        /// Adds a component by emplacing.
        template<typename T, typename... Args>
        void Add(Args&&... args)
        {
            ThrowsIfInvalid();
            auto component_id = ComponentManager::instance().id<T>();
            entity_manager_->AddComponent<T>(index_, component_id, std::forward<Args>(args)...);
        }

        /// Adds a component by copying or moving.
        template<typename T>
        void AddFrom(T&& val)
        {
            ThrowsIfInvalid();
            using ValT = typename std::remove_reference<T>::type;
            auto component_id = ComponentManager::instance().id<ValT>();
            entity_manager_->AddComponent<ValT>(index_, component_id, std::forward<T>(val));
        }

        /// Removes the component.
        template<typename T>
        void Remove()
        {
            ThrowsIfInvalid();
            auto component_id = ComponentManager::instance().id<T>();
            entity_manager_->RemoveComponent(index_, component_id);
        }

        /// Gets a component pointer.
        ///
        /// This pointer is valid until this component is removed or this entity is destroyed.
        template<typename T>
        T* Get()
        {
            ThrowsIfInvalid();
            auto component_id = ComponentManager::instance().id<T>();
            return static_cast<T*>(entity_manager_->GetComponent(index_, component_id));
        }

        // component access without type
        
        /// Adds a component by copying without type.
        void AddFrom(const std::string& component_name, const void * value)
        {
            ThrowsIfInvalid();
            auto component_id = ComponentManager::instance().id(component_name);
            entity_manager_->AddComponentFrom(index_, component_id, value);
        }

        /// Adds a component by moving without type.
        void AddFromMove(const std::string& component_name, void * value)
        {
            ThrowsIfInvalid();
            auto component_id = ComponentManager::instance().id(component_name);
            entity_manager_->AddComponentFromMove(index_, component_id, value);
        }

        /// Removes a component without type.
        void Remove(const std::string& component_name)
        {
            ThrowsIfInvalid();
            auto component_id = ComponentManager::instance().id(component_name);
            entity_manager_->RemoveComponent(index_, component_id);
        }

        /// Gets a component pointer without type.
        ///
        /// This pointer is valid until this component is removed or this entity is destroyed.
        void* Get(const std::string& component_name)
        {
            ThrowsIfInvalid();
            auto component_id = ComponentManager::instance().id(component_name);
            return entity_manager_->GetComponent(index_, component_id);
        }

        // operators

        bool operator==(const EntityHandle & rhs) const
        {
            return entity_manager_ == rhs.entity_manager_ && index_ == rhs.index_ && version_ == rhs.version_;
        }

        bool operator!=(const EntityHandle & rhs) const
        {
            return !operator==(rhs);
        }

    private:

        friend World;
        friend View;

        // don't call any member functions
        EntityHandle() :
            entity_manager_(nullptr),
            index_(0),
            version_(0)
        {}

        EntityHandle(EntityManager & entity_manager, std::uint32_t index, std::uint32_t version) :
            entity_manager_(&entity_manager),
            index_(index),
            version_(version)
        {}

        void ThrowsIfInvalid() const
        {
            if (!valid())
            {
                throw std::logic_error("The entity handle " + std::to_string(id()) + " is invalid");
            }
        }

        EntityManager * entity_manager_;
        std::uint32_t index_;
        std::uint32_t version_;
    };
}
