#pragma once

#include <spdlog/spdlog.h>
#include <string_view>

namespace ff::log {
enum class log_level {
    Fatal = 0,
    Error = 1,
    Warning = 2,
    Info = 3,
    Debug = 4,
    Trace = 5
};

class Logger {
public:
    // Only declarations here (implemented in logger.cpp)
    static void initialize();
    static void shutdown();

    // Templates must be inline in the header so the compiler can expand them
    template<typename... Args>
    static void print(log_level level, fmt::format_string<Args...> fmt_str, Args&&... args) {
        switch (level) {
            case log_level::Fatal:   spdlog::critical(fmt_str, std::forward<Args>(args)...); break;
            case log_level::Error:   spdlog::error(fmt_str, std::forward<Args>(args)...); break;
            case log_level::Warning: spdlog::warn(fmt_str, std::forward<Args>(args)...); break;
            case log_level::Info:    spdlog::info(fmt_str, std::forward<Args>(args)...); break;
            case log_level::Debug:   spdlog::debug(fmt_str, std::forward<Args>(args)...); break;
            case log_level::Trace:   spdlog::trace(fmt_str, std::forward<Args>(args)...); break;
        }
    }
};

} // namespace ff::log

// --- Macro Definitions ---
#define initialize_logging() ff::log::Logger::initialize()
#define shutdown_logging()   ff::log::Logger::shutdown()
#define log_output(level, ...) ff::log::Logger::print(level, __VA_ARGS__)

#ifndef FERROR
#define FERROR(...)                 log_output(ff::log::log_level::Error, __VA_ARGS__)
#define FWARN(...)                  log_output(ff::log::log_level::Warning, __VA_ARGS__)
#define FINFO(...)                  log_output(ff::log::log_level::Info, __VA_ARGS__)
#define FDEBUG(...)                 log_output(ff::log::log_level::Debug, __VA_ARGS__)
#define FERROR_STR(msg)             log_output(ff::log::log_level::Error, "{}", msg)
#endif