#include "engine.hpp"

#include "events/event_bus.hpp"
#include "events/window_events.hpp"
#include "gfx/opengl.hpp"
#include "logging/engine_logger.hpp"
#include "logging/macros.hpp"
#include "rendering/renderer.hpp"
#include "strings.hpp"
#include "windowing/glfw.hpp"

using namespace kuiper;

engine::engine_or_err engine::initialise(std::unique_ptr<kuiper::application> app, const config& config) {
    auto& logger = engine_logger::get();

    logger.info("Initialising {}", strings::engine_name);

    // Initialise GLFW

    const auto maybe_window = glfw_window::initialise_context_and_window(app->get_app_spec(), config.backend);
    if (const auto* err = std::get_if<error>(&maybe_window)) {
        return *err;
    }
    auto window = std::get<std::shared_ptr<kuiper::glfw_window>>(maybe_window);

    // Initialise OpenGL

    if (const auto err = opengl::initialise()) {
        return *err;
    }

    const auto [width, height]    = window->framebuffer_size();
    const auto [x_scale, y_scale] = window->content_scale();
    opengl::set_viewport_dimensions(0, 0, width, height);

    ENGINE_LOG_DEBUG("Initial window size: {} x {}, content scale: {:.2f} x {:.2f}", width, height, x_scale, y_scale);
    (void) x_scale;
    (void) y_scale;

    // Initialise renderer

    auto renderer = renderer::initialise();
    if (const auto* err = std::get_if<error>(&renderer)) {
        return *err;
    }

    return engine {std::move(app),
                   std::get<std::shared_ptr<kuiper::glfw_window>>(maybe_window),
                   std::get<std::shared_ptr<kuiper::renderer>>(renderer)};
}

void engine::shutdown() {
    glfw_window::shutdown();
    opengl::shutdown();

    return;
}

window::s_ptr engine::window() {
    return m_window;
}

std::vector<scene>& engine::scenes() {
    return m_scenes;
}

error::maybe engine::run() {
    // Post-initialisation configuration

    auto event_bus = event_bus::get();

    const auto subscription_id = event_bus.subscribe<framebuffer_resized_event>([](const framebuffer_resized_event& e) {
        opengl::set_viewport_dimensions(0, 0, e.width, e.height);
    });

    // Allow client application to initialise

    {
        auto app_logger = std::make_unique<logger>(m_app->get_app_spec().name.c_str());
        m_app->initialise(this, std::move(app_logger));
    }

    while (!m_window->should_close() && !m_app->should_quit()) {
        event_bus.poll();

        m_app->pre_render(this, 0.0f);

        m_renderer->render(m_scenes);

        m_app->post_render(this, 0.0f);

        m_window->swap_buffers();
        m_window->poll_events();
    }

    // Notify application we're terminating

    m_app->shutdown(this);

    // Unsubscribe from events

    event_bus.unsubscribe<framebuffer_resized_event>(subscription_id);

    return std::nullopt;
}
