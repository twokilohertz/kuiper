#pragma once

#include "error.hpp"
#include "gfx/object.hpp"
#include "gfx/utils.hpp"

#include "resources/image.hpp"

#include <glad/gl.h>

#include <cmath>
#include <expected>
#include <variant>

namespace kuiper {

namespace opengl {

    class texture : public object {
      public:
        texture(std::uint32_t name)
            : object(name) {}

      public:
        using texture_or_err = std::expected<texture, error>;

      public:
        static texture_or_err create(std::uint32_t target) {
            GLuint new_name = 0;
            glCreateTextures(target, 1, &new_name);

            if (!new_name) {
                if (const auto err = opengl::get_last_error()) {
                    return std::unexpected(*err);
                }
            }

            return texture(new_name);
        }

      public:
        void destroy() {
            return object::destroy(glDeleteTextures);
        }

        void bind(std::uint32_t target) {
            return object::bind(glBindTexture, target);
        }

        error::maybe set_storage(std::uint32_t      width,
                                 std::uint32_t      height,
                                 sized_pixel_format format = sized_pixel_format::rgba8) {
            const std::uint32_t max_texture_levels = std::floor(std::log2(std::max(width, height))) + 1;

            glTextureStorage2D(name(), max_texture_levels, pixfmt_to_sized_enum(format), width, height);

            if (const auto err = opengl::get_last_error()) {
                return *err;
            }

            return std::nullopt;
        }

        error::maybe upload(const std::uint8_t* data,
                            std::uint32_t       width,
                            std::uint32_t       height,
                            sized_pixel_format  format = sized_pixel_format::rgba8) {
            constexpr int base_level = 0;

            glTextureSubImage2D(name(),
                                base_level,
                                0,
                                0,
                                width,
                                height,
                                pixfmt_to_format_enum(format),
                                pixfmt_to_type_enum(format),
                                data);

            if (const auto err = opengl::get_last_error()) {
                return *err;
            }

            glGenerateTextureMipmap(name());

            if (const auto err = opengl::get_last_error()) {
                return *err;
            }

            return std::nullopt;
        }

        error::maybe set_parameter(GLenum param, std::int32_t value) {
            glTextureParameteri(name(), param, value);

            if (const auto err = opengl::get_last_error()) {
                return *err;
            }

            return std::nullopt;
        }
    };

} // namespace opengl

} // namespace kuiper
