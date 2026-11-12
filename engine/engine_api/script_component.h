#pragma once

#include "components/components_common.h"

namespace ff::components {
DEFINE_TYPED_ID(ScriptId);

class ScriptComponent final {
  public:
    constexpr explicit ScriptComponent(ScriptId id) : mId{ id } {}
    constexpr ScriptComponent() : mId{ id::invalid_id } {}

    constexpr ScriptId get_id() const { return mId; }
    constexpr bool     is_valid() const { return id::is_valid(mId); }

  private:
    ScriptId mId;
};
} // namespace ff::components