#pragma once

#include "components_common.h"

namespace ff {
#define INIT_INFO(component)                                                                                 \
    namespace components {                                                                                   \
    struct component##InitInfo;                                                                              \
    }

// forward declare any init infos used by entity for entity creation
INIT_INFO(TransformComponent)
// INIT_INFO(script);
#undef INIT_INFO

namespace entity {
constexpr u64 INITIAL_CAPACITY = 1024;
struct EntityInfo {
    components::TransformComponentInitInfo *transform{ nullptr };
};

// TODO: add create, remove, is_alive functions for entity::Entity -> should be implemented under
// engine_api/entity_component.h

Entity create_entity(const EntityInfo &info);
void   remove_entity(EntityId id);

bool   is_alive(EntityId id);
} // namespace entity

} // namespace ff