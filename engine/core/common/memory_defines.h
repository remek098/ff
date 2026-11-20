#pragma once

#include <memory>

namespace ff {

template <typename T, typename Deleter = std::default_delete<T>>
using uptr = std::unique_ptr<T, Deleter>;

template <typename T>
using sptr = std::shared_ptr<T>;

// Optional convenience helpers matching std::make_unique and std::make_shared
template <typename T, typename... Args>
constexpr uptr<T> make_uptr(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
constexpr sptr<T> make_sptr(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace ff