#include "windowing/glfw.hpp"

#include "application.hpp"
#include "error.hpp"
#include "events/event_bus.hpp"
#include "events/window_events.hpp"
#include "logging/engine_logger.hpp"
#include "logging/macros.hpp"

#include <GLFW/glfw3.h>

#include <thread>

using namespace kuiper;

glfw_window::window_or_err glfw_window::initialise_context_and_window(const kuiper::application_spec& app_spec,
                                                                      std::optional<window::backend>  backend) {
    glfwSetErrorCallback(glfw_window::error_callback); // can be registered before window is created

    if (backend) {
        switch (*backend) {
        case window::backend::x11:
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
            break;

        case window::backend::wayland:
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
            break;
        }
    } else {
        // TODO: move away from X11 by default - sort out the wayland scaling issues!
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    }

    if (!glfwInit()) {
        return error {.code = error_code::window_init_failed, .description = "failed to initialise GLFW"};
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

#if !defined(NDEBUG)
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(app_spec.width, app_spec.height, app_spec.name.c_str(), nullptr, nullptr);
    if (!window) {
        return error {.code = error_code::window_init_failed, .description = "failed to create GLFW window"};
    }

    make_context_current(window);
    register_callbacks(window);

    ENGINE_LOG_DEBUG(
        "Initialised GLFW {} and created window \"{}\"", glfwGetVersionString(), glfwGetWindowTitle(window));

    auto ret = std::make_shared<glfw_window>(window);

    glfwSetWindowUserPointer(window, ret.get());

    return ret;
}

void glfw_window::destroy_window(GLFWwindow* window) {
    ENGINE_LOG_DEBUG("Shutting down GLFW window \"{}\" & context ...", glfwGetWindowTitle(window));

    unregister_callbacks(window);

    glfwSetErrorCallback(nullptr);
    glfwDestroyWindow(window);

    return;
}

void glfw_window::shutdown() {
    return glfwTerminate();
}

void* glfw_window::native_handle() {
    return m_window;
}

bool glfw_window::should_close() {
    return glfwWindowShouldClose(m_window);
}

void glfw_window::swap_buffers() {
    return glfwSwapBuffers(m_window);
}

void glfw_window::set_title(const char* title) {
    return glfwSetWindowTitle(m_window, title);
}

const char* glfw_window::title() const {
    return glfwGetWindowTitle(m_window);
}

void glfw_window::set_icon(const std::uint8_t* data, std::uint32_t width, std::uint32_t height) {
    const GLFWimage image {
        .width = static_cast<int>(width), .height = static_cast<int>(height), .pixels = (unsigned char*) data};

    return glfwSetWindowIcon(m_window, 1, &image);
}

void glfw_window::set_swap_interval(vsync_mode mode) {
    switch (mode) {
    case window::vsync_mode::off:
        return glfwSwapInterval(-1);
    case window::vsync_mode::on:
        return glfwSwapInterval(1);
    case window::vsync_mode::double_buffered:
        return glfwSwapInterval(2);
    }

    return;
}

void glfw_window::poll_events() {
    return glfwPollEvents();
}

std::tuple<std::uint32_t, std::uint32_t> glfw_window::framebuffer_size() {
    int width  = 0;
    int height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);

    return {width, height};
}

std::tuple<float, float> glfw_window::content_scale() {
    float x_scale = 1.0f;
    float y_scale = 1.0f;
    glfwGetWindowContentScale(m_window, &x_scale, &y_scale);

    return {x_scale, y_scale};
}

void glfw_window::make_context_current(GLFWwindow* window) {
    ENGINE_LOG_DEBUG("Making OpenGL context current on thread {} for window \"{}\"",
                     std::this_thread::get_id(),
                     glfwGetWindowTitle(window));

    return glfwMakeContextCurrent(window);
}

void glfw_window::register_callbacks(GLFWwindow* window) {
    ENGINE_LOG_DEBUG("Registering callbacks for window \"{}\"", glfwGetWindowTitle(window));

    glfwSetFramebufferSizeCallback(window, glfw_window::framebuffer_size_callback);
    glfwSetWindowContentScaleCallback(window, glfw_window::content_scale_callback);

    return;
}

void glfw_window::unregister_callbacks(GLFWwindow* window) {
    // should be reverse order of register_callbacks()
    ENGINE_LOG_DEBUG("Unregistering callbacks for window \"{}\"", glfwGetWindowTitle(window));

    glfwSetWindowContentScaleCallback(window, nullptr);
    glfwSetFramebufferSizeCallback(window, nullptr);

    return;
}

void glfw_window::error_callback(int code, const char* description) {
    auto& logger = engine_logger::get();
    logger.error("GLFW error (code: {}): {}", code, description);
    return;
}

void glfw_window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    const glfw_window* instance = (const glfw_window*) glfwGetWindowUserPointer(window);

    const std::uint32_t w = static_cast<std::uint32_t>(width);
    const std::uint32_t h = static_cast<std::uint32_t>(height);

    auto& logger = engine_logger::get();
    logger.trace("Window \"{}\" framebuffer resized: {} x {}", instance->title(), w, h);

    event_bus::get().publish<event::framebuffer_resized>({w, h});

    return;
}

void glfw_window::content_scale_callback(GLFWwindow* window, float x_scale, float y_scale) {
    const glfw_window* instance = (const glfw_window*) glfwGetWindowUserPointer(window);

    auto& logger = engine_logger::get();
    logger.trace("Window \"{}\" content scale changed: {:.2f} x {:.2f}", instance->title(), x_scale, y_scale);

    return;
}

void glfw_window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void) window;
    (void) key;
    (void) scancode;
    (void) action;
    (void) mods;

    return;
}
