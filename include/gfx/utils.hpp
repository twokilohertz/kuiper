#pragma once

#include <glad/gl.h>

#include "error.hpp"
#include <optional>

namespace kuiper {

namespace opengl {
    constexpr const char* error_to_string(GLenum error_code);
    std::optional<error>  get_last_error();

    // Data types & formats

    enum class sized_pixel_format {
        rgba8
    };

    GLenum pixfmt_to_sized_enum(sized_pixel_format format);
    GLenum pixfmt_to_format_enum(sized_pixel_format format);
    GLenum pixfmt_to_type_enum(sized_pixel_format format);

}; // namespace opengl

} // namespace kuiper
