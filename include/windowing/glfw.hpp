#pragma once

#include "application.hpp"
#include "windowing/window.hpp"

#include "error.hpp"

#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <variant>

namespace kuiper {

class glfw_window : public window {
  public:
    glfw_window(GLFWwindow* window)
        : m_window(window) {}

    ~glfw_window() {
        destroy_window(m_window);
    }

  public:
    using s_ptr         = std::shared_ptr<glfw_window>;
    using window_or_err = std::variant<s_ptr, error>;

  public:
    static window_or_err initialise_context_and_window(const kuiper::application_spec& app_spec,
                                                       std::optional<window::backend>  backend = std::nullopt);
    static void          destroy_window(GLFWwindow* window);
    static void          shutdown();
    static void          make_context_current(GLFWwindow* window);

  public:
    virtual void*       native_handle() override;
    virtual bool        should_close() override;
    virtual void        swap_buffers() override;
    virtual void        set_title(const char* title) override;
    virtual const char* title() const override;
    virtual void        set_icon(const std::uint8_t* data, std::uint32_t width, std::uint32_t height) override;
    virtual void        set_swap_interval(vsync_mode mode) override;
    virtual void        poll_events() override;

    virtual std::tuple<std::uint32_t, std::uint32_t> framebuffer_size() override;
    virtual std::tuple<float, float>                 content_scale() override;

  private:
    static void register_callbacks(GLFWwindow* window);
    static void unregister_callbacks(GLFWwindow* window);

  private:
    static void error_callback(int code, const char* description);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void content_scale_callback(GLFWwindow* window, float x_scale, float y_scale);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

  private:
    GLFWwindow* m_window {nullptr};
};

} // namespace kuiper
