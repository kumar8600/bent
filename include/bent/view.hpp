#pragma once

#include <bitset>

#include "internal/definitions.hpp"
#include "internal/world_state.hpp"
#include "entity_handle.hpp"

namespace bent
{
    struct World;

    struct View
    {
        struct iterator;

        iterator begin()
        {
            return iterator(*world_state_, world_state_->entities_.begin(), world_state_->entities_.end(), component_mask_);
        }

        iterator end()
        {
            return iterator(*world_state_, world_state_->entities_.end(), world_state_->entities_.end(), component_mask_);
        }

        struct iterator : std::iterator<std::forward_iterator_tag, EntityHandle>
        {
            iterator& operator++()
            {
                do
                {
                    ++it_;
                } while (it_ != end_ && (it_->marked_as_garbage || !Match(it_->component_mask, component_mask_)));
                if (it_ != end_)
                {
                    entity_handle_ = EntityHandle(it_->id, *world_state_, WorldState::TemporalEntityHandle(it_ - world_state_->entities_.begin()));
                }
                return *this;
            }

            iterator operator++(int)
            {
                auto tmp = *this;
                operator++();
                return tmp;
            }

            reference operator*()
            {
                return entity_handle_;
            }

            pointer operator->()
            {
                return std::addressof(operator*());
            }

            bool operator==(const iterator & rhs) const
            {
                return it_ == rhs.it_;
            }

            bool operator!=(const iterator & rhs) const
            {
                return !operator==(rhs);
            }

        private:
            friend View;

            iterator(WorldState & world_state, WorldState::EntityContainer::iterator it, WorldState::EntityContainer::iterator end, const std::bitset<MAX_COMPONENTS> & component_mask) :
                world_state_(&world_state),
                it_(it),
                end_(end),
                entity_handle_(world_state),
                component_mask_(component_mask)
            {
                while (it_ != end_ && (it_->marked_as_garbage || !Match(it_->component_mask, component_mask_)))
                {
                    ++it_;
                }
                if (it_ != end_)
                {
                    entity_handle_ = EntityHandle(it_->id, *world_state_, WorldState::TemporalEntityHandle(it_ - world_state_->entities_.begin()));
                }
            }

            static bool Match(const std::bitset<MAX_COMPONENTS> & component_mask, const std::bitset<MAX_COMPONENTS> & query)
            {
                auto masked = component_mask & query;
                return masked == query;
            }

            WorldState * world_state_;
            WorldState::EntityContainer::iterator it_;
            WorldState::EntityContainer::iterator end_;
            EntityHandle entity_handle_;
            std::bitset<MAX_COMPONENTS> component_mask_;
        };

    private:
        friend World;

        View(WorldState & world_state, const std::bitset<MAX_COMPONENTS> & component_mask) :
            world_state_(&world_state),
            component_mask_(component_mask)
        {
        }

        WorldState * world_state_;
        std::bitset<MAX_COMPONENTS> component_mask_;
    };
}
