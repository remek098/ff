#pragma once

#include "components/components_common.h"

#include "utils/glm_math.h"

namespace ff::components {
DEFINE_TYPED_ID(TransformId);

class TransformComponent final {
  public:
    constexpr explicit TransformComponent(TransformId id) : mId{ id } {}
    constexpr TransformComponent() : mId{ id::invalid_id } {}

    constexpr TransformId get_id() const { return mId; }
    constexpr bool        is_valid() const { return id::is_valid(mId); }

    glm::quat get_rotation() const;
    glm::vec3 get_orientation() const;
    glm::vec3 get_position() const;
    glm::vec3 get_scale() const;


  private:
    TransformId mId;
};
} // namespace ff