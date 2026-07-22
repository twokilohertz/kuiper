#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

namespace kuiper {

class window {
  public:
    window()          = default;
    virtual ~window() = default;

  public:
    using s_ptr = std::shared_ptr<window>;

    enum class vsync_mode {
        off,
        on,
        double_buffered,
    };

    enum class backend {
        x11,
        wayland
    };

  public:
    virtual void* native_handle() = 0;

    virtual bool        should_close()                                                         = 0;
    virtual void        swap_buffers()                                                         = 0;
    virtual void        set_swap_interval(vsync_mode mode)                                     = 0;
    virtual void        set_title(const char* title)                                           = 0;
    virtual const char* title() const                                                          = 0;
    /// data - Pointer to RGBA8 image data
    virtual void set_icon(const std::uint8_t* data, std::uint32_t width, std::uint32_t height) = 0;
    virtual void poll_events()                                                                 = 0;

    virtual std::tuple<std::uint32_t, std::uint32_t> framebuffer_size() = 0;
    virtual std::tuple<float, float>                 content_scale()    = 0;
};
} // namespace kuiper
