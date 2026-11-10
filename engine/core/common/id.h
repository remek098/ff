#pragma once

#include "common_headers.h"

#include <type_traits>

namespace ff::id {
using Id = u32;

namespace detail {
// index part gives us the index of the entity in lookup array.
// generation part is used to distinguish entities created at the same index slot

constexpr u32 generation_bits{ 10 };
constexpr u32 index_bits{ sizeof(Id) * 8 - generation_bits };

// mask to be able to retrieve information
constexpr Id generation_mask{ (Id{ 1 } << generation_bits) - 1 };
constexpr Id index_mask{ (Id{ 1 } << index_bits) - 1 };
} // namespace detail

constexpr Id  invalid_id{ Id(-1) };
constexpr u32 minimum_deleted_elements{ 1024 };

using generation_t = std::conditional_t<detail::generation_bits <= 16,
                                        std::conditional_t<detail::generation_bits <= 8, u8, u16>,
                                        u32>;
static_assert(sizeof(generation_t) * 8 >= detail::generation_bits,
              "detail::generation_bits exceeds the bit capacity of the chosen generation type!");
static_assert((sizeof(Id) - sizeof(generation_t)) > 0,
              "The id::generation_t type is too large! It leaves 0 bytes for the entity index "
              "inside the id.");

constexpr bool
is_valid(Id id) {
    return id != invalid_id;
}

constexpr Id
index(Id id) {
    Id index{ id & detail::index_mask };
    FASSERT(index != detail::index_mask,
            "id::index(Id): Attempting to extract an index from an invalid or uninitialized entity "
            "ID! You might want to check if id::is_valid(Id)");
    return index;
}

constexpr Id
generation(Id id) {
    return (id >> detail::index_bits) & detail::generation_mask;
}

constexpr Id
new_generation(Id id) {
    const Id generation{ id::generation(id) + 1 };
    // check if we don't exceed the max amount this unsigned type can hold -> e.g.
    // u8 -> 255
    // u16 -> 65535
    // u32 -> 4294967295

    // -1 because we count generations from 0. We have to check this
    FASSERT(generation < (((u64)1 << detail::generation_bits) - 1),
            "Id::new_generation(Id): New generation would exceed max generation count for current "
            "setup.");
    return index(id) | (generation << detail::index_bits);
}

#if FF_IS_DEBUG
namespace detail {
struct IdBase {
    constexpr explicit IdBase(Id id) : m_id{ id } {}

    // for doing e.g. u32 id = ...
    constexpr operator Id() const { return m_id; }

  private:
    Id m_id;
};
} // namespace detail

// one constructor takes value, other initializes with invalid index.
#define DEFINE_TYPED_ID(name)                                                                      \
    struct name final : id::detail::IdBase {                                                       \
        constexpr explicit name(id::Id id) : IdBase{ id } {}                                       \
                                                                                                   \
        constexpr name() : IdBase{ 0 } {}                                                          \
    };
#else
#define DEFINE_TYPED_ID(name) using name = id::Id;
#endif

} // namespace ff::id