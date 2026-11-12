#include "entity_component.h"
#include "transform_component.h"
#include "script_component.h"

namespace ff::entity {
namespace {
std::vector<components::TransformComponent> transforms;
std::vector<components::ScriptComponent>    scripts;

std::vector<id::generation_t>               generations;
std::deque<EntityId>                        free_ids;
} // namespace

// TODO: adding and removing script components from entity.

Entity
create_entity(const EntityInfo &info) {
    FASSERT_MSG(info.transform,
                "No InitInfo for Transform component passed for entity. That's a must have one. Returning "
                "default-initialized entity...");
    if (!info.transform) return Entity{};

    EntityId id;

    // either reuse free existing id or add a new slot for it at the end of the array
    if (free_ids.size() > id::minimum_deleted_elements) {
        // pick the first one, check if it's one of those 'dead' entities
        id = free_ids.front();
        FASSERT_MSG(!is_alive(id), "Trying to reuse non-dead Entity.");
        // remove it from free_ids, increase it's generation
        free_ids.pop_front();
        id = EntityId{ id::new_generation(id) };
        ++generations[id::index(id)];
    } else {
        // if we dind't run out of space for our entities yet, just add new entity to our vector and set it's
        // generation to 0
        id = EntityId{ (id::Id)generations.size() };
        generations.push_back(0);

        // NOTE: don't call resize(), so that the number of memory allocations stays as low as possible
        transforms.emplace_back();
        scripts.emplace_back();
    }

    const Entity new_entity{ id };
    const id::Id index{ id::index(id) };

    // create transform component
    assert(!transforms[index].is_valid());
    transforms[index] = components::create_transform_component(*info.transform, new_entity);
    if (!transforms[index].is_valid()) return {};

    if (info.script && info.script->script_creator) {
        FASSERT(!scripts[index].is_valid());
        scripts[index] = components::create_script_component(*info.script, new_entity);
        FASSERT(scripts[index].is_valid());
    }

    return new_entity;
}

void
remove_entity(EntityId id) {
    const id::Id index{ id::index(id) };
    FASSERT_MSG(is_alive(id), "Trying to remove already removed Entity");

    if (scripts[index].is_valid()) {
        components::remove_script_component(scripts[index]);
        scripts[index] = {}; // empty component with INVALID_ID
    }

    components::remove_transform_component(transforms[index]);
    transforms[index] = {};

    free_ids.push_back(id);
}

bool
is_alive(EntityId id) {
    FASSERT_MSG(id::is_valid(id), "");
    const id::Id index{ id::index(id) };

    FASSERT_MSG(index < generations.size(),
                "Id of checked if alive entity crossed past the biggest generation available.");
    return (generations[index] == id::generation(id) && transforms[index].is_valid());
}

components::TransformComponent
Entity::get_transform() const {
    FASSERT_MSG(is_alive(m_id), "Cannot get TransformComponent from non-alive Entity.");
    const id::Id index{ id::index(m_id) };
    return transforms[index];
}

components::ScriptComponent
Entity::get_script() const {
    FASSERT_MSG(is_alive(m_id), "Cannot get ScriptComponent from non-alive Entity.");
    const id::Id index{ id::index(m_id) };
    return scripts[index];
}

} // namespace ff::entity