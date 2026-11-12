#include "script_component.h"
#include "entity_component.h"
#include "transform_component.h"

#include "utils/utilities.h"

#define USE_TRANSFORM_CACHE_MAP 0

namespace ff {
namespace {
std::vector<script::ScriptPtr>   script_entities;
std::vector<id::Id>              id_mapping;
std::vector<id::generation_t>    generations;
std::deque<components::ScriptId> free_ids;

// so that we can track all changed to entities that have their transform changed (changed are cached and then at proper
// time in a frame applied) those hold changes that happen in the game scripts.
std::vector<components::TransformComponentCache> transform_cache;
#if USE_TRANSFORM_CACHE_MAP
std::unordered_map<id::Id, u32> cache_map;
#endif

using ScriptRegistry = std::unordered_map<size_t, script::ScriptCreatorFnp>;

ScriptRegistry &
get_script_registery() {
    // NOTE: putting this static variable in function to avoid shenanigans with
    // initialization order of static data.
    static ScriptRegistry reg;
    return reg;
}

#ifdef USE_WITH_EDITOR
std::vector<std::string> &
get_script_names() {
    // NOTE: putting this static variable in function to avoid shenanigans with
    // initialization order of static data.
    static std::vector<std::string> names;
    return names;
}
#endif

bool
script_exists(components::ScriptId id) {
    FASSERT_MSG(id::is_valid(id), "ScriptId not valid");
    const id::Id index{ id::index(id) };
    FASSERT(index < generations.size() && id_mapping[index] < script_entities.size());
    FASSERT(generations[index] == id::generation(id));

    return (generations[index] == id::generation(id)) && script_entities[id_mapping[index]]
           && script_entities[id_mapping[index]]->is_valid();
}

#if USE_TRANSFORM_CACHE_MAP

components::TransformComponentCache *const
get_transform_component_cache_ptr(const entity::Entity *const entt) {
    FASSERT(entity::is_alive((*entt).get_id()));
    const components::TransformId id{ (*entt).get_transform().get_id() };

    u32                           index{ u32_invalid_id };
    auto                          pair = cache_map.try_emplace(id, id::invalid_id);

    if (pair.second) {
        index = (u32)transform_cache.size();
        transform_cache.emplace_back();
        transform_cache.back().id = id;
        cache_map[id]             = index;
    } else {
        index = cache_map[id];
    }

    FASSERT(index < transform_cache.size());
    return &transform_cache[index];
}
#else
// simple linear search inside cache array. To avoid allocations from std::unordered_map
components::TransformComponentCache *const
get_transform_component_cache_ptr(const entity::Entity *const entt) {
    FASSERT(entity::is_alive((*entt).get_id()));
    const components::TransformId id{ (*entt).get_transform().get_id() };

    for (auto &cache : transform_cache) {
        if (cache.id == id) {
            return &cache;
        }
    }

    transform_cache.emplace_back();
    transform_cache.back().id = id;

    return &transform_cache.back();
}
#endif
} // namespace

namespace script {
u8
register_script(size_t tag, ScriptCreatorFnp func) {
    bool result = get_script_registery().insert(ScriptRegistry::value_type{ tag, func }).second;
    FASSERT(result);
    return result;
}

ScriptCreatorFnp
get_script_creator(size_t tag) {
    auto script = ff::get_script_registery().find(tag);
    FASSERT(script != ff::get_script_registery().end() && script->first == tag);
    return script->second;
}

#ifdef USE_WITH_EDITOR
u8
add_script_name(const char *name) {
    get_script_names().emplace_back(name);
    return true;
}
#endif

void
ScriptEntity::set_rotation(const entity::Entity *const entity, glm::quat rotation_quaternion) {
    components::TransformComponentCache& cache{*get_transform_component_cache_ptr(entity)};
    cache.flags |= components::TransformComponentFlags::ROTATION;
    cache.rotation = rotation_quaternion;
}

void
ScriptEntity::set_orientation(const entity::Entity *const entity, glm::vec3 orientation_vector) {
    components::TransformComponentCache& cache{*get_transform_component_cache_ptr(entity)};
    cache.flags |= components::TransformComponentFlags::ORIENTATION;
    cache.rotation = orientation_vector;
}

void
ScriptEntity::set_position(const entity::Entity *const entity, glm::vec3 position) {
    components::TransformComponentCache& cache{*get_transform_component_cache_ptr(entity)};
    cache.flags |= components::TransformComponentFlags::POSITION;
    cache.rotation = position;
}

void
ScriptEntity::set_scale(const entity::Entity *const entity, glm::vec3 scale) {
    components::TransformComponentCache& cache{*get_transform_component_cache_ptr(entity)};
    cache.flags |= components::TransformComponentFlags::SCALE;
    cache.rotation = scale;
}

} // namespace script

namespace components {

ScriptComponent
create_script_component(const ScriptComponentInitInfo &info, entity::Entity entt) {
    FASSERT(entt.is_valid());
    FASSERT(info.script_creator);

    ScriptId id;
    if (free_ids.size() > id::minimum_deleted_elements) {
        id = free_ids.front();
        FASSERT(!script_exists(id)); // assert that id doesn't exist
        // free, reuse by increasing generation for that id slot
        free_ids.pop_front();
        id = ScriptId{ id::new_generation(id) };
        ++generations[id::index(id)];
    } else {
        // that means we got to add new slots at the end of arrays
        id = ScriptId{ (id::Id)id_mapping.size() };
        id_mapping.emplace_back();
        generations.push_back(0);
    }

    FASSERT(id::is_valid(id));
    // get position of where new script_entity was added, ofc it is end of script_entities array
    const id::Id index{ (id::Id)script_entities.size() };
    script_entities.emplace_back(info.script_creator(entt));
    FASSERT(script_entities.back()->get_id() == entt.get_id()); // should always be true
    id_mapping[id::index(id)] = index;
    FDEBUG("Script created for entity with id: {}", (u32)id);
    return ScriptComponent{ id };
}

void
remove_script_component(ScriptComponent component) {
    FASSERT(component.is_valid() && script_exists(component.get_id()));
    const ScriptId id{ component.get_id() };
    const id::Id   index{ id_mapping[id::index(id)] };
    // get id of last script.
    const ScriptId last_id{ script_entities.back()->get_script().get_id() };
    utils::erase_unordered(script_entities, index);
    id_mapping[id::index(last_id)] = index;
    id_mapping[id::index(id)]      = id::invalid_id;
}

void
update_script_component(f32 dt) {
    for (auto &ptr : script_entities) {
        ptr->update(dt);
    }

    // after all scripts have been executed we can send the cached updates to the transform component module and
    // reset our cache array and prepare for the next frame.
    if (transform_cache.size()) {
        components::update_tranform_component(transform_cache.data(), (u32)transform_cache.size());
        transform_cache.clear();

#if USE_TRANSFORM_CACHE_MAP
        cache_map.clear();
#endif
    }
}
} // namespace components

} // namespace ff
