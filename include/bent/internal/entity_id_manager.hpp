#pragma once

#include "definitions.hpp"

namespace bent
{
    struct EntityIdManager
    {
        entity_id_t Create()
        {
            ++greatest_id_ever_; // id begins from 1
            return greatest_id_ever_;
        }

        void Destroy(entity_id_t)
        {
            // do nothing
        }

    private:
        entity_id_t greatest_id_ever_ = 0;
    };
}
