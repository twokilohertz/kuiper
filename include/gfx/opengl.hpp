#pragma once

#include "error.hpp"

#include <glad/gl.h>

#include <cstdint>
#include <optional>
#include <type_traits>

namespace kuiper {

namespace opengl {
    constexpr const char* glsl_version = "#version 460 core";

    error::maybe initialise();
    void         shutdown();

    void set_clear_colour(float red, float green, float blue, float alpha);
    void clear_framebuffer(std::optional<std::uint32_t> clear_bits = std::nullopt);
    void set_viewport_dimensions(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height);

    template<typename T>
    T get_parameter(GLenum param_name) {
        T value {};

        if (std::is_same_v<T, GLboolean>) {
            glGetBooleanv(param_name, &value);
        } else if (std::is_same_v<T, GLdouble>) {
            glGetDoublev(param_name, &value);
        } else if (std::is_same_v<T, GLfloat>) {
            glGetFloatv(param_name, &value);
        } else if (std::is_same_v<T, GLint>) {
            glGetIntegerv(param_name, &value);
        } else if (std::is_same_v<T, GLint64>) {
            glGetInteger64v(param_name, &value);
        }

        return value;
    }
}; // namespace opengl

} // namespace kuiper
