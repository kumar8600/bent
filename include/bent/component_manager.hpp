#pragma once

#include <string>
#include <memory>
#include <deque>
#include <unordered_map>
#include <typeindex>
#include <limits>
#include <stdexcept>

#include "internal/definitions.hpp"
#include "internal/dynamic_region_allocator.hpp"

namespace bent
{
    //
    // Declarations
    //

    /// Singleton class that manages component types.
    struct ComponentManager
    {
        ComponentManager(const ComponentManager&) = delete;
        ComponentManager& operator=(const ComponentManager&) = delete;

        static ComponentManager& instance();

        template<typename T>
        component_type_id_t RegisterComponent(const std::string& name);

        template <typename T>
        component_type_id_t id();
        component_type_id_t id(const std::string& name) const;
        std::string name(component_type_id_t id) const;
        kumar::DynamicRegionAllocatorInterface & allocator(component_type_id_t id) const;

        component_type_id_t size() const;

    private:
        ComponentManager()
        {}

        std::unordered_map<std::string, component_type_id_t> id_by_name_;
        std::unordered_map<component_type_id_t, std::string> name_by_id_;
        std::unordered_map<std::type_index, component_type_id_t> id_by_type_;
        std::deque<std::unique_ptr<kumar::DynamicRegionAllocatorInterface>> allocator_by_id_;
        component_type_id_t size_ = 0; // id is start from 0.
    };

    template <typename T>
    void RegisterComponent(const std::string& name);

    //
    // Definitions
    //

    inline ComponentManager & ComponentManager::instance()
    {
        static ComponentManager instance;
        return instance;
    }

    template<typename T>
    inline component_type_id_t ComponentManager::RegisterComponent(const std::string & name)
    {
        if (size_ == MAX_COMPONENTS)
        {
            throw std::out_of_range("too many components are registered (hint: configure <bent/defintions.hpp>)");
        }
        auto i = id<T>();

        auto res = name_by_id_.emplace(i, name);
        if (!res.second)
        {
            throw std::out_of_range("the component `" + name + "` has already registered as `" + res.first->second + "`");
        }
        id_by_name_.emplace(name, i);

        return i;
    }

    template <typename T>
    inline component_type_id_t ComponentManager::id()
    {
        auto it = id_by_type_.find(typeid(T));
        if (it != id_by_type_.end())
        {
            return it->second;
        }
        else
        {
            auto id = size_;
            id_by_type_.emplace(typeid(T), id);
            allocator_by_id_.emplace_back(new kumar::DynamicRegionAllocator<T>);

            ++size_;
            return id;
        }
    }

    inline component_type_id_t ComponentManager::id(const std::string & name) const
    {
        return id_by_name_.at(name);
    }

    inline std::string ComponentManager::name(component_type_id_t id) const
    {
        return name_by_id_.at(id);
    }

    inline kumar::DynamicRegionAllocatorInterface & ComponentManager::allocator(component_type_id_t id) const
    {
        return *allocator_by_id_[id];
    }

    inline component_type_id_t ComponentManager::size() const
    {
        return size_;
    }

    template<typename T>
    void RegisterComponent(const std::string & name)
    {
        auto& instance = ComponentManager::instance();
        instance.RegisterComponent<T>(name);
    }
}
