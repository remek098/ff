#include "entity_component.h"
#include "transform_component.h"

namespace ff::entity {
namespace {
std::vector<components::TransformComponent> transforms;

std::vector<id::generation_t>               generations;
std::deque<EntityId>                        free_ids;
} // namespace

Entity
create_entity(const EntityInfo &info) {
    FASSERT(info.transform,
            "No InitInfo for Transform component passed for entity. That's a must have one. Returning default-initialized entity...");
    if (!info.transform) return Entity{};

    EntityId id;

    // either reuse free existing id or add a new slot for it at the end of the array
    if (free_ids.size() > id::minimum_deleted_elements) {
        // pick the first one, check if it's one of those 'dead' entities
        id = free_ids.front();
        FASSERT(!is_alive(id), "Trying to reuse non-dead Entity.");
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
    }

    const Entity new_entity{ id };
    const id::Id index{ id::index(id) };

    // create transform component
    assert(!transforms[index].is_valid());
    transforms[index] = components::create_transform_component(*info.transform, new_entity);
    if (!transforms[index].is_valid()) return {};

    return new_entity;
}

void
remove_entity(EntityId id) {
    const id::Id index{ id::index(id) };
    FASSERT(is_alive(id), "Trying to remove already removed Entity");

    components::remove_transform_component(transforms[index]);
    transforms[index] = {};

    free_ids.push_back(id);
}

bool
is_alive(EntityId id) {
    FASSERT(id::is_valid(id), "");
    const id::Id index{ id::index(id) };

    FASSERT(index < generations.size(), "Id of checked if alive entity crossed past the biggest generation available.");
    return (generations[index] == id::generation(id) && transforms[index].is_valid());
}

components::TransformComponent
Entity::get_transform() const {
    FASSERT(is_alive(m_id), "Cannot get TransformComponent from non-alive Entity.");
    const id::Id index{ id::index(m_id) };
    return transforms[index];
}

} // namespace ff::entity