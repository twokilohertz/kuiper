#pragma once

#include "error.hpp"
#include "gfx/object.hpp"
#include "gfx/utils.hpp"

#include <glad/gl.h>

#include <variant>

namespace kuiper {

namespace opengl {

    class framebuffer : public object {
      public:
        framebuffer(std::uint32_t name)
            : object(name) {}

      public:
        using framebuffer_or_err = std::variant<framebuffer, error>;

      public:
        static framebuffer_or_err create() {
            GLuint new_name = 0;
            glCreateFramebuffers(1, &new_name);

            if (!new_name) {
                if (const auto err = opengl::get_last_error()) {
                    return *err;
                }
            }

            return framebuffer(new_name);
        }

      public:
        void destroy() {
            return object::destroy(glDeleteFramebuffers);
        }

        void bind(std::uint32_t target) {
            return object::bind(glBindFramebuffer, target);
        }
    };

} // namespace opengl

} // namespace kuiper
