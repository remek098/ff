#pragma once

#include "components/components_common.h"
#include "engine_api/entity_component.h"

namespace ff {
namespace entity {
DEFINE_TYPED_ID(EntityId);
class Entity {
  public:
    constexpr explicit Entity(EntityId id) : m_id{ id } {}
    // kinda unnecessary if we default initialize it to invalid_id, but still better to have it just in case
    constexpr Entity() : m_id{ id::invalid_id } {}

    [[nodiscard]] constexpr EntityId get_id() const { return m_id; }
    [[nodiscard]] constexpr bool     is_valid() const { return id::is_valid(m_id); }

  private:
    EntityId m_id{ id::invalid_id };
};
} // namespace entity
} // namespace ff