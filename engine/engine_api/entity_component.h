#pragma once

#include "components/components_common.h"
#include "transform_component.h"

namespace ff {
namespace entity {
DEFINE_TYPED_ID(EntityId);
class Entity {
  public:
    constexpr explicit Entity(EntityId id) : m_id{ id } {}
    // kinda unnecessary if we default initialize it to invalid_id, but still better to have it just in case
    constexpr Entity() : m_id{ id::invalid_id } {}

    [[nodiscard]] constexpr EntityId             get_id() const { return m_id; }
    [[nodiscard]] constexpr bool                 is_valid() const { return id::is_valid(m_id); }

    [[nodiscard]] components::TransformComponent get_transform() const;

    [[nodiscard]] glm::quat                      rotation() const { return get_transform().get_rotation(); }
    [[nodiscard]] glm::vec3                      orientation() const { return get_transform().get_orientation(); }
    [[nodiscard]] glm::vec3                      position() const { return get_transform().get_position(); }
    [[nodiscard]] glm::vec3                      scale() const { return get_transform().get_scale(); }

  private:
    EntityId m_id{ id::invalid_id };
};
} // namespace entity
} // namespace ff