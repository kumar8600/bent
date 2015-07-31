#pragma once

#include <iterator>

#include "internal/entity_manager.hpp"
#include "entity_handle.hpp"

namespace bent
{
    struct World;

    struct View
    {
        struct iterator : std::iterator<std::forward_iterator_tag, EntityHandle>
        {
            reference operator*()
            {
                return entity_handle_;
            }

            pointer operator->()
            {
                return &entity_handle_;
            }

            iterator & operator++()
            {
                ++index_;
                next();
                return *this;
            }

            iterator operator++(int)
            {
                auto tmp = *this;
                operator++();
                return tmp;
            }

            bool operator==(const iterator& rhs) const
            {
                return index_ == rhs.index_;
            }

            bool operator!=(const iterator& rhs) const
            {
                return !operator==(rhs);
            }

        private:
            friend View;
            using ComponentMask = EntityManager::ComponentMask;
            using EntityVersionVectorIterator = EntityManager::EntityVersionVector::iterator;

            iterator(EntityManager & entity_manager, const ComponentMask & component_mask, std::uint32_t index, std::uint32_t end) :
                entity_manager_(&entity_manager),
                component_mask_(component_mask),
                index_(index),
                end_(end)
            {
                next();
            }

            void next()
            {
                while (true)
                {
                    if (index_ == end_)
                    {
                        return;
                    }
                    if (entity_manager_->alive(index_) && (entity_manager_->component_mask(index_) & component_mask_) == component_mask_)
                    {
                        break;
                    }
                    ++index_;
                }
                entity_handle_ = EntityHandle(*entity_manager_, index_, entity_manager_->version(index_));
            }

            EntityManager * entity_manager_;
            ComponentMask component_mask_;
            std::uint32_t index_;
            std::uint32_t end_;
            EntityHandle entity_handle_;
        };

        iterator begin()
        {
            return iterator(*entity_manager_, component_mask_, 0, entity_manager_->entity_versions_.size());
        }

        iterator end()
        {
            return iterator(*entity_manager_, component_mask_, entity_manager_->entity_versions_.size(), entity_manager_->entity_versions_.size());
        }

    private:
        friend World;
        using ComponentMask = EntityManager::ComponentMask;

        View(EntityManager & entity_manager, const ComponentMask & component_mask) :
            entity_manager_(&entity_manager),
            component_mask_(component_mask)
        {
        }

        EntityManager * entity_manager_;
        ComponentMask component_mask_;
    };
}
