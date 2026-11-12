#pragma once

#include "components_common.h"

namespace ff::components {
struct ScriptComponentInitInfo {
    script::ScriptCreatorFnp script_creator;
};

ScriptComponent create_script_component(const ScriptComponentInitInfo &info, entity::Entity entt);
void            remove_script_component(ScriptComponent component);
void            update_script_component(f32 dt);

} // namespace ff::components