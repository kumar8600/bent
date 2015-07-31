#pragma once

#include <string>
#include <memory>
#include <deque>
#include <unordered_map>
#include <typeindex>
#include <limits>
#include <stdexcept>

#include "internal/definitions.hpp"
#include "internal/dynamic_constructor.hpp"
#include "internal/component_pool_factory.hpp"

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
        std::uint16_t RegisterComponent(const std::string& name);

        template <typename T>
        std::uint16_t id();
        std::uint16_t id(const std::string& name) const;
        std::string name(std::uint16_t id) const;
        DynamicConstructorInterface & dynamic_constructor(std::uint16_t id) const;
        ComponentPoolFactoryInterface & component_pool_factory(std::uint16_t id) const;

        std::uint16_t size() const;

    private:
        ComponentManager()
        {}

        std::unordered_map<std::string, std::uint16_t> id_by_name_;
        std::unordered_map<std::uint16_t, std::string> name_by_id_;
        std::unordered_map<std::type_index, std::uint16_t> id_by_type_;
        std::deque<std::unique_ptr<DynamicConstructorInterface>> dynamic_constructor_by_id_;
        std::deque<std::unique_ptr<ComponentPoolFactoryInterface>> component_pool_factory_by_id_;
        std::uint16_t size_ = 0; // id is start from 0.
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
    inline std::uint16_t ComponentManager::RegisterComponent(const std::string & name)
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
    inline std::uint16_t ComponentManager::id()
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
            dynamic_constructor_by_id_.emplace_back(new DynamicConstructor<T>);
            component_pool_factory_by_id_.emplace_back(new ComponentPoolFactory<T>);

            ++size_;
            return id;
        }
    }

    inline std::uint16_t ComponentManager::id(const std::string & name) const
    {
        return id_by_name_.at(name);
    }

    inline std::string ComponentManager::name(std::uint16_t id) const
    {
        return name_by_id_.at(id);
    }

    inline DynamicConstructorInterface & ComponentManager::dynamic_constructor(std::uint16_t id) const
    {
        return *dynamic_constructor_by_id_[id];
    }

    inline ComponentPoolFactoryInterface & ComponentManager::component_pool_factory(std::uint16_t id) const
    {
        return *component_pool_factory_by_id_[id];
    }

    inline std::uint16_t ComponentManager::size() const
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
