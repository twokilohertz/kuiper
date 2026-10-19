#include "gfx/opengl.hpp"

#include "error.hpp"
#include "logging/macros.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <optional>

using namespace kuiper;

error::maybe opengl::initialise() {
    // GLFW must have been initialised successfully and a window (+ context) created by this point

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        return error {.code = error_code::gfx_init_failed, .description = "failed to resolve OpenGL function pointers"};
    }

    ENGINE_LOG_DEBUG("Initialised OpenGL {}.{} using GLAD (generator version {})",
                     GLAD_VERSION_MAJOR(version),
                     GLAD_VERSION_MINOR(version),
                     GLAD_GENERATOR_VERSION);

    return std::nullopt;
}

void opengl::shutdown() {
    return;
}

void opengl::set_clear_colour(float red, float green, float blue, float alpha) {
    return glClearColor(red, green, blue, alpha);
}

void opengl::clear_framebuffer(std::optional<std::uint32_t> clear_bits) {
    if (clear_bits.has_value()) {
        return glClear(clear_bits.value());
    } else [[unlikely]] {
        std::uint32_t clear_flags = 0;

        {
            GLint params = 0;
            glGetFramebufferAttachmentParameteriv(
                GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &params);

            if (params != GL_NONE) {
                clear_flags |= GL_COLOR_BUFFER_BIT;
            }
        }

        {
            GLint params = 0;
            glGetFramebufferAttachmentParameteriv(
                GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &params);

            if (params != GL_NONE) {
                clear_flags |= GL_DEPTH_BUFFER_BIT;
            }
        }

        {
            GLint params = 0;
            glGetFramebufferAttachmentParameteriv(
                GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &params);

            if (params != GL_NONE) {
                clear_flags |= GL_STENCIL_BUFFER_BIT;
            }
        }

        return glClear(clear_flags);
    }
}
void opengl::set_viewport_dimensions(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height) {
    return glViewport(x, y, width, height);
}
