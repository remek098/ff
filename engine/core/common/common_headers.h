#pragma once

// --- Auto-detect Build Configuration ---
#if defined(_DEBUG) || defined(DEBUG)
// Explicitly set by MSVC Debug configurations or manual flags
#define FF_IS_DEBUG 1
#elif defined(NDEBUG)
// Standard release flag present
#define FF_IS_DEBUG 0
#elif defined(__OPTIMIZE__)
// GCC/Clang optimization flags are active (-O1, -O2, etc.) -> Treat as Release
#define FF_IS_DEBUG 0
#else
// Absolute fallback: If compiling raw without any flags, default to Debug for safety
#define FF_IS_DEBUG 1
#endif

#ifndef DISABLE_COPY
#define DISABLE_COPY(T)                                                                                                \
    explicit T(const T &)   = delete;                                                                                  \
    T &operator=(const T &) = delete;
#endif // !DISABLE_COPY

#ifndef DISABLE_MOVE
#define DISABLE_MOVE(T)                                                                                                \
    explicit T(T &&)   = delete;                                                                                       \
    T &operator=(T &&) = delete;
#endif // !DISABLE_COPY

#ifndef DISABLE_COPY_AND_MOVE
#define DISABLE_COPY_AND_MOVE(T)                                                                                       \
    DISABLE_COPY(T)                                                                                                    \
    DISABLE_MOVE(T)
#endif // !DISABLE_COPY_AND_MOVE

#define DEFAULT_WINDOW_WIDTH  1280
#define DEFAULT_WINDOW_HEIGHT 720

#include "core/logging/logger.h"
#include "primitive_types.h"

#if defined(_MSC_VER)
#define FDEBUGBREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
#define FDEBUGBREAK() __builtin_debugtrap()
#else
#include <cstdlib>
#define FDEBUGBREAK() std::abort()
#endif

#ifndef FASSERT_MSG
#if FF_IS_DEBUG
#define FASSERT_MSG(condition, ...)                                                                                    \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            FERROR("Assertion failed: {0}", #condition);                                                               \
            if constexpr (sizeof("" __VA_ARGS__) > 1) {                                                                \
                FERROR(__VA_ARGS__);                                                                                   \
            }                                                                                                          \
            FDEBUGBREAK();                                                                                             \
        }                                                                                                              \
    } while (0)

#define FASSERT(condition) FASSERT_MSG(condition, "")

// #define FASSERT(condition)                                                                                             \
//     do {                                                                                                               \
//         if (!(condition)) {                                                                                            \
//             FERROR("Assertion failed: {0}", #condition);                                                               \
//             FDEBUGBREAK();                                                                                             \
//         }                                                                                                              \
//     } while (0)
#else
#define FASSERT_MSG(condition, ...) ((void)0)
#define FASSERT(condition)          ((void)0)
#endif // !FF_IS_DEBUG
#endif // !FASSERT_MSG

#include "id.h"
