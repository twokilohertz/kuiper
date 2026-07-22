#pragma once

#include "strings.hpp"

#include <optional>
#include <string>

namespace kuiper {

enum class error_code {
    no_error = 0,
    cli_args_invalid,
    window_init_failed,
    gfx_init_failed,
    renderer_init_failed,
    opengl_error,
    unknown
};

constexpr const char* error_code_to_string(error_code code) {
    switch (code) {
    case error_code::no_error:
        return "No error";
    case error_code::cli_args_invalid:
        return "Command line arguments invalid";
    case error_code::window_init_failed:
        return "Window initialisation failed";
    case error_code::gfx_init_failed:
        return "Graphics initialisation failed";
    case error_code::renderer_init_failed:
        return "Renderer initialisation failed";
    case error_code::opengl_error:
        return "OpenGL error";
    case error_code::unknown:
        return "Unknown error";
    }

    return strings::missing_string;
}

class error {
  public:
    using maybe = std::optional<error>;

  public:
    error_code  code {error_code::unknown};
    std::string description {};
};

} // namespace kuiper
