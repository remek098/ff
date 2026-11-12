#pragma once

#include "components/components_common.h"
#include "transform_component.h"
#include "script_component.h"

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
    [[nodiscard]] components::ScriptComponent    get_script() const;

    [[nodiscard]] glm::quat                      rotation() const { return get_transform().get_rotation(); }
    [[nodiscard]] glm::vec3                      orientation() const { return get_transform().get_orientation(); }
    [[nodiscard]] glm::vec3                      position() const { return get_transform().get_position(); }
    [[nodiscard]] glm::vec3                      scale() const { return get_transform().get_scale(); }

  private:
    EntityId m_id{ id::invalid_id };
};
} // namespace entity

namespace script {
class ScriptEntity : public entity::Entity {
  public:
    virtual ~ScriptEntity() = default;
    virtual void begin_play() {}
    virtual void update(f32) {}

  protected:
    constexpr explicit ScriptEntity(entity::Entity entity) : entity::Entity{ entity.get_id() } {}

    void        set_rotation(glm::quat rotation_quaternion) { set_rotation(this, rotation_quaternion); }
    void        set_orientation(glm::vec3 orientation_vector) { set_orientation(this, orientation_vector); }
    void        set_position(glm::vec3 position) { set_position(this, position); }
    void        set_scale(glm::vec3 scale) { set_scale(this, scale); }

    static void set_rotation(const entity::Entity *const entity, glm::quat rotation_quaternion);
    static void set_orientation(const entity::Entity *const entity, glm::vec3 orientation_vector);
    static void set_position(const entity::Entity *const entity, glm::vec3 position);
    static void set_scale(const entity::Entity *const entity, glm::vec3 scale);
};

using ScriptPtr        = std::unique_ptr<ScriptEntity>;
using ScriptCreatorFnp = ScriptPtr (*)(entity::Entity entity);
using StringHash       = std::hash<std::string>;

u8               register_script(size_t, ScriptCreatorFnp);

ScriptCreatorFnp get_script_creator(size_t tag);

template <class ScriptClass>
ScriptPtr
create_script(entity::Entity entt) {
    FASSERT_MSG(entt.is_valid(), "Cannot create script for invalid entity.");
    return std::make_unique<ScriptClass>(entt);
}

#ifdef USE_WITH_EDITOR
u8 add_script_name(const char *name);

#define REGISTER_SCRIPT(TYPE)                                                                                          \
    namespace {                                                                                                        \
    const u8 _reg_##TYPE{ ff::script::register_script(ff::script::StringHash()(#TYPE),                                 \
                                                      &ff::script::create_script<TYPE>) };                             \
    const u8 _name_##TYPE{ ff::script::add_script_name(#TYPE) };                                                       \
    }
// end of REGISTER_SCRIPT macro
#else
#define REGISTER_SCRIPT(TYPE)                                                                                          \
    namespace {                                                                                                        \
    const u8 _reg_##TYPE{ ff::script::register_script(ff::script::StringHash()(#TYPE),                                 \
                                                      &ff::script::create_script<TYPE>) };                             \
    }
// end of REGISTER_SCRIPT macro

#endif // USE_WITH_EDITOR

} // namespace script
} // namespace ff