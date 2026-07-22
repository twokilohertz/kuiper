#pragma once

#include "spdlog/common.h"
#include <format>
#include <memory>
#include <utility>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace kuiper {

class logger {
  public:
    enum class level {
        off,
        critical,
        error,
        warning,
        info,
        debug,
        trace
    };

#if !defined(NDEBUG)
    static constexpr level default_level = level::debug;
#else
    static constexpr level default_level = level::info;
#endif

  public:
    logger(const char* name = nullptr, logger::level level = default_level)
        : m_log(spdlog::stdout_color_mt(name)) {
        set_level(level);
    }

    ~logger() = default;

    template<typename... Args>
    inline void trace(std::format_string<Args...> fmt, Args&&... args) {
        m_log->trace(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void debug(std::format_string<Args...> fmt, Args&&... args) {
        m_log->debug(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void info(std::format_string<Args...> fmt, Args&&... args) {
        m_log->info(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void warn(std::format_string<Args...> fmt, Args&&... args) {
        m_log->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void error(std::format_string<Args...> fmt, Args&&... args) {
        m_log->error(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void critical(std::format_string<Args...> fmt, Args&&... args) {
        m_log->critical(fmt, std::forward<Args>(args)...);
    }

    inline void set_level(logger::level level) {
        switch (level) {
        case level::off:
            m_log->set_level(spdlog::level::off);
            break;
        case level::critical:
            m_log->set_level(spdlog::level::critical);
            break;
        case level::error:
            m_log->set_level(spdlog::level::err);
            break;
        case level::warning:
            m_log->set_level(spdlog::level::warn);
            break;
        case level::info:
            m_log->set_level(spdlog::level::info);
            break;
        case level::debug:
            m_log->set_level(spdlog::level::debug);
            break;
        case level::trace:
            m_log->set_level(spdlog::level::trace);
            break;
        }
    }

  private:
    std::shared_ptr<spdlog::logger> m_log {nullptr};
};

} // namespace kuiper
