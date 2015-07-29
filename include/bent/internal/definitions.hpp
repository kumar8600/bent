#pragma once

#include <cstdint>
#include <limits>

namespace bent
{
    using entity_id_t = std::uint64_t;
    using component_type_id_t = std::uint16_t;
    using temporal_entity_id_t = std::uint64_t;
    using state_version_t = std::uint64_t;
    constexpr entity_id_t INVALID_ENTITY_ID = 0;
    constexpr state_version_t INVALID_STATE_VERSION = std::numeric_limits<state_version_t>::max();
    constexpr component_type_id_t MAX_COMPONENTS = 256;
}
