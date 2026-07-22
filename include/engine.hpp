#pragma once

#include "error.hpp"

#include "application.hpp"
#include "configuration.hpp"
#include "rendering/renderer.hpp"
#include "rendering/scene.hpp"
#include "windowing/window.hpp"

#include <utility>
#include <variant>
#include <vector>

namespace kuiper {

class engine {
  public:
    engine() = delete;
    engine(kuiper::application::u_ptr app, kuiper::window::s_ptr window, kuiper::renderer::s_ptr renderer)
        : m_app(std::move(app)), m_window(window), m_renderer(renderer) {}

  public:
    using engine_or_err = std::variant<engine, error>;

  public:
    static engine_or_err initialise(kuiper::application::u_ptr app, const config& config);

  public:
    error::maybe run();
    void         shutdown();

    kuiper::window::s_ptr window();
    std::vector<scene>&   scenes();

  private:
    // Core
    kuiper::application::u_ptr m_app {nullptr};
    kuiper::window::s_ptr      m_window {nullptr};

    // Rendering
    kuiper::renderer::s_ptr m_renderer {nullptr};
    std::vector<scene>      m_scenes {};
};

} // namespace kuiper
