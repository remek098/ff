#include "transform_component.h"
#include "entity_component.h"

namespace ff::components {
namespace {
std::vector<glm::mat4x4> to_world;
std::vector<glm::mat4x4> inv_world;
std::vector<glm::quat>   rotations;
std::vector<glm::vec3>   orientations;
std::vector<glm::vec3>   positions;
std::vector<glm::vec3>   scales;
std::vector<u8>          has_transforms;              // to find out which entities have their transform matrices already calculated
std::vector<u8>          changes_from_previous_frame; // remember flags for each entity so that we know if it has been
                                                      // moved, rotated or scaled
u8                       read_write_flag;             // indicates when we need to reset changes_from_previous_frame array when we enter the
                                                      // writing phase of the frame.

void
ensure_capacity_for_index(u64 entity_index) {
    if (entity_index >= positions.size()) {
        u64 new_size = std::max(positions.size() * 2, entity_index + 1);

        positions.resize(new_size);
        rotations.resize(new_size);
        orientations.resize(new_size);
        scales.resize(new_size);
        to_world.resize(new_size);
        inv_world.resize(new_size);
        has_transforms.resize(new_size, 0);
        changes_from_previous_frame.resize(new_size, (u8)TransformComponentFlags::ALL);
    }
}

void
calculate_transform_matrices(id::Id index) {
    FASSERT(rotations.size() > index, "Internal arrays for TransformComponent have lower size than passed index.");
    FASSERT(positions.size() > index, "Internal arrays for TransformComponent have lower size than passed index.");
    FASSERT(scales.size() > index, "Internal arrays for TransformComponent have lower size than passed index.");

    // Build SRT (Scale -> Rotate -> Translate) world matrix
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), positions[index]);
    glm::mat4 rotation    = glm::mat4_cast(rotations[index]);
    glm::mat4 scale       = glm::scale(glm::mat4(1.0f), scales[index]);

    glm::mat4 world       = translation * rotation * scale;
    to_world[index]       = world;

    // Inverse world for transforming vectors without translation
    // (stripping translation row/column or building rotation-scale inverse)
    glm::mat4 world_no_translation = rotation * scale;
    inv_world[index]               = glm::inverse(world_no_translation);

    has_transforms[index]          = 1;
}

glm::vec3
calculate_orientation(glm::quat rotation) {
    const glm::vec3 front(0.0f, 0.0f, -1.0f);

    return rotation * front;
}

void
set_rotation(TransformId id, const glm::quat &rotation_quaternion) {
    const u32 index{ id::index(id) };
    rotations[index]      = rotation_quaternion;
    orientations[index]   = calculate_orientation(rotation_quaternion);
    has_transforms[index] = 0; // so that transform matrices will also be recalculated when someone asks for them
    changes_from_previous_frame[index] |= TransformComponentFlags::ROTATION; // set update flag for rotation
}

void
set_orientation(TransformId, const glm::vec3 &) {}

void
set_position(TransformId id, const glm::vec3 &position) {
    const u32 index{ id::index(id) };
    positions[index]      = position;
    has_transforms[index] = 0; // so that transform matrices will also be recalculated when someone asks for them
    changes_from_previous_frame[index] |= TransformComponentFlags::POSITION; // set update flag for position
}

void
set_scale(TransformId id, const glm::vec3 &scale) {
    const u32 index{ id::index(id) };
    scales[index]         = scale;
    has_transforms[index] = 0; // so that transform matrices will also be recalculated when someone asks for them
    changes_from_previous_frame[index] |= TransformComponentFlags::SCALE; // set update flag for scale
}
} // namespace

TransformComponent
create_transform_component(const TransformComponentInitInfo &info, entity::Entity entt) {
    FASSERT(entt.is_valid(), "Cannot create TransformComponent for invalid entity.");
    const id::Id entity_index{ id::index(entt.get_id()) };

    ensure_capacity_for_index(entity_index); // makes sure std::vectors are resized properly
    // when entity_index is pointing inside our array, we fill one of holes in array
    glm::quat rotation{ info.rotation };
    rotations[entity_index]                   = rotation;
    orientations[entity_index]                = calculate_orientation(rotation);
    positions[entity_index]                   = glm::vec3{ info.position };
    scales[entity_index]                      = glm::vec3{ info.scale };
    has_transforms[entity_index]              = 0;
    changes_from_previous_frame[entity_index] = (u8)TransformComponentFlags::ALL;

    // NOTE: each entity has a transform component. Therefore ids for transform components are exactly the
    // same as entity ids.
    return TransformComponent(TransformId{ entt.get_id() });
}

void
remove_transform_component(TransformComponent component) {
    // NOTE: technically we don't have to clean up positions, rotations and scales utl::vectors there,
    // since create() manages allocation quite well.
    FASSERT(component.is_valid(), "TransformComponent not valid during remove.");
    const u32 index{ id::index(component.get_id()) };

    has_transforms[index]              = 0;
    changes_from_previous_frame[index] = 0;

    positions[index]                   = glm::vec3(0.0f);
    rotations[index]                   = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    scales[index]                      = glm::vec3(1.0f);
    to_world[index]                    = glm::mat4(1.0f);
    inv_world[index]                   = glm::mat4(1.0f);
}

void
get_transform_matrices(const entity::EntityId id, glm::mat4x4 &world, glm::mat4x4 &inverse_world) {
    FASSERT(entity::Entity{ id }.is_valid(), "Trying to get transform matrices from invalid Entity");

    const id::Id entity_index{ id::index(id) };
    if (!has_transforms[entity_index]) {
        calculate_transform_matrices(entity_index);
    }

    world         = to_world[entity_index];
    inverse_world = inv_world[entity_index];
}

void
get_updated_transform_component_flags(const entity::EntityId *const ids, u32 count, u8 *const flags) {
    FASSERT(ids && count && flags, "One of params (or all of them) are invalid or wrong.");
    read_write_flag = 1; // mark that we read data

    for (u32 i = 0; i < count; ++i) {
        FASSERT(entity::Entity{ ids[i] }.is_valid(), "Read Entity with id {} is not valid.", id::index(ids[i]));
        flags[i] = changes_from_previous_frame[id::index(ids[i])];
    }
}

void
update_tranform_component(const TransformComponentCache *const cache, u32 count) {
    FASSERT(cache && count, "Invalid params for update_transform_component()");
    // NOTE: clearing changes_from_previous_frame happens once every frame when there will be no reads
    //       and the caches are about to be applied by calling this function (i.e. the rest of the current frame will only have writes).
    if (read_write_flag) {
        // u8 is 1 byte in size so skipping being verbose about it.
        memset(changes_from_previous_frame.data(), 0, changes_from_previous_frame.size());
        read_write_flag = 0;
    }

    for (u32 i{ 0 }; i < count; ++i) {
        const TransformComponentCache &c{ cache[i] };
        FASSERT(TransformComponent{ c.id }.is_valid(), "Invalid cache during update_transform_component()");

        if (c.flags & TransformComponentFlags::ROTATION) set_rotation(c.id, c.rotation);
        if (c.flags & TransformComponentFlags::ORIENTATION) set_orientation(c.id, c.orientation);
        if (c.flags & TransformComponentFlags::POSITION) set_position(c.id, c.position);
        if (c.flags & TransformComponentFlags::SCALE) set_scale(c.id, c.scale);
    }
}

glm::quat
TransformComponent::get_rotation() const {
    FASSERT(is_valid(), "TransformId not valid for requested entity.:", (u32)get_id());
    return rotations[id::index(mId)];
}

glm::vec3
TransformComponent::get_orientation() const {
    FASSERT(is_valid(), "TransformId not valid for requested entity.:", (u32)get_id());
    return orientations[id::index(mId)];
}
glm::vec3
TransformComponent::get_position() const {
    FASSERT(is_valid(), "TransformId not valid for requested entity.:", (u32)get_id());
    return positions[id::index(mId)];
}
glm::vec3
TransformComponent::get_scale() const {
    FASSERT(is_valid(), "TransformId not valid for requested entity.:", (u32)get_id());
    return scales[id::index(mId)];
}

} // namespace ff::components
