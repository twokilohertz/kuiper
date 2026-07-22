#include "configuration.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <ranges>
#include <string_view>

using namespace kuiper;

std::optional<window::backend> parse_windowing_backend(const char* arg) {
    if (!arg)
        return std::nullopt;

    const std::size_t len = std::strlen(arg);

    if (!len)
        return std::nullopt;

    std::string_view arg_string(arg, len);

    if (arg_string == "x11") {
        return window::backend::x11;
    } else if (arg_string == "wayland" || arg_string == "wl") {
        return window::backend::wayland;
    }

    return std::nullopt;
}

config config::from_args_and_env(std::span<const char* const> args) {
    config cfg {};

    // CLI args

    for (const auto [idx, arg] : std::views::enumerate(args)) {
        const std::size_t len = std::strlen(arg);

        if (!len)
            continue;

        std::string_view arg_string(arg, len);

        if (arg_string == "-w") {
            const std::size_t next_idx = (idx + 1);

            // all this could be args.at(next_idx) in C++26 :/
            if (next_idx >= args.size())
                continue;
            const auto next_arg = *(args.cbegin() + next_idx);

            if (const auto w = parse_windowing_backend(next_arg)) {
                cfg.backend = *w;
            }
        } else if (arg_string == "-v") {
            cfg.verbosity = verbosity::verbose;
        } else if (arg_string == "-vv") {
            cfg.verbosity = verbosity::very_verbose;
        }
    }

    // Environment variables (takes precedence over CLI args)

    if (const char* windowing_env = std::getenv("KUIPER_WINDOWING")) {
        if (const auto w = parse_windowing_backend(windowing_env)) {
            cfg.backend = *w;
        }
    }

    return cfg;
}
