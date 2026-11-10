#pragma once

#include "components_common.h"

namespace ff {
#define INIT_INFO(component)                                                                                 \
    namespace component {                                                                                    \
    struct init_info;                                                                                        \
    }

// forward declare any init infos used by entity for entity creation
// INIT_INFO(transform);
// INIT_INFO(script);
#undef INIT_INFO

namespace entity {
struct init_info {};
} // namespace entity

// TODO: add create, remove, is_alive functions for entity::Entity -> should be implemented under
// engine_api/entity_component.h

} // namespace ff