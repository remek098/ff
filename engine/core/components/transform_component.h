#pragma once

#include "components_common.h"

namespace ff::components {
struct TransformComponentInitInfo {
    glm::quat rotation{};
    glm::vec3 position{};
    glm::vec3 scale{};
};

struct TransformComponentFlags {
    enum flags : u32 {
        ROTATION    = 0x01,
        ORIENTATION = 0x02,
        POSITION    = 0x04,
        SCALE       = 0x08,

        ALL         = ROTATION | ORIENTATION | POSITION | SCALE
    };
};

struct TransformComponentCache {
    glm::quat   rotation;
    glm::vec3   orientation;
    glm::vec3   position;
    glm::vec3   scale;
    TransformId id;
    u32         flags; // use components::TransformComponentFlags::flags
};

TransformComponent create_transform_component(const TransformComponentInitInfo &info, entity::Entity entt);
void               remove_transform_component(TransformComponent component);

void get_transform_matrices(const entity::EntityId id, glm::mat4x4& world, glm::mat4x4& inverse_world);

void get_updated_transform_component_flags(const entity::EntityId *const ids, u32 count, u8 *const flags);
void update_tranform_component(const TransformComponentCache *const cache, u32 count);
} // namespace ff::components