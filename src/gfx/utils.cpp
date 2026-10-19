#include "gfx/utils.hpp"

#include "strings.hpp"

using namespace kuiper;

constexpr const char* opengl::error_to_string(GLenum error_code) {
    switch (error_code) {
    case GL_NO_ERROR:
        return "No error has been recorded";
    case GL_INVALID_ENUM:
        return "An unacceptable value is specified for an enumerated argument";
    case GL_INVALID_VALUE:
        return "A numeric argument is out of range";
    case GL_INVALID_OPERATION:
        return "The specified operation is not allowed in the current state";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "The framebuffer object is not complete";
    case GL_OUT_OF_MEMORY:
        return "There is not enough memory left to execute the command";
    case GL_STACK_UNDERFLOW:
        return "An attempt has been made to perform an operation that would cause an internal stack to underflow";
    case GL_STACK_OVERFLOW:
        return "An attempt has been made to perform an operation that would cause an internal stack to overflow";
    }

    return "";
}

std::optional<error> opengl::get_last_error() {
    error err {.code = error_code::no_error};

    while (auto next_error = glGetError()) {
        err.code        = error_code::opengl_error;
        err.description = error_to_string(next_error);
    }

    if (err.code == error_code::no_error) {
        return std::nullopt;
    } else {
        return err;
    }
}

GLenum opengl::pixfmt_to_sized_enum(sized_pixel_format format) {
    switch (format) {
    case sized_pixel_format::rgba8:
        return GL_RGBA8;
    }

    return 0;
}

GLenum opengl::pixfmt_to_format_enum(sized_pixel_format format) {
    switch (format) {
    case sized_pixel_format::rgba8:
        return GL_RGBA;
    }

    return 0;
}

GLenum opengl::pixfmt_to_type_enum(sized_pixel_format format) {
    switch (format) {
    case sized_pixel_format::rgba8:
        return GL_UNSIGNED_BYTE;
    }

    return 0;
}
