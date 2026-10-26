#include "logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <vector>
#include <memory>

namespace ff::log {
void Logger::initialize() {
    try {
        // Heavy spdlog setup happens exactly ONCE here in this translation unit
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("console.log", true);

        console_sink->set_pattern("[%^%l%$]: %v");
        file_sink->set_pattern("[%l]: %v");

        std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };
        auto logger = std::make_shared<spdlog::logger>("RD_ENGINE", sinks.begin(), sinks.end());

        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::trace);
    } 
    catch (const spdlog::spdlog_ex& ex) {
        // Handle exception
    }
}

void Logger::shutdown() {
    spdlog::shutdown();
}

} // namespace ff::log