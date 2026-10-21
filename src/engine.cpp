#include "engine.hpp"

#include "events/event_bus.hpp"
#include "events/window_events.hpp"
#include "gfx/opengl.hpp"
#include "logging/engine_logger.hpp"
#include "logging/macros.hpp"
#include "rendering/renderer.hpp"
#include "strings.hpp"
#include "windowing/glfw.hpp"

#include "ecs/ecs.hpp"
#include "ecs/sparse_map.hpp"
#include "ecs/sparse_set.hpp"

#include "maths/transform.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>

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

    // nonsense

    ecs::sparse_set<std::uint32_t> s;
    assert(!s.contains(0));
    s.insert(0);
    s.insert(1);
    s.insert(2);
    assert(s.contains(0) && s.contains(1) && s.contains(2));
    s.erase(1);
    assert(!s.contains(1));
    s.insert(1);
    assert(s.contains(1));

    ecs::sparse_map<ecs::entity_t, std::string> m;
    m.insert(ecs::entity_t::pack(0, 67), "hi");
    m.insert(ecs::entity_t::pack(1, 67), "hello");
    m.insert(ecs::entity_t::pack(2, 67), "how are you");
    m.insert(ecs::entity_t::pack(3, 67), "nice to see you");
    m.erase(ecs::entity_t::pack(2, 67));
    assert(!m.empty());

    assert(ecs::entity_t::pack(1337, 67).unpack().first == 1337);
    assert(ecs::entity_t::pack(0, 67).unpack().second == 67);

    for (const auto& [x, y] : m) {
        ENGINE_LOG_DEBUG("key: {}, value: {}", x.id(), y);
    }

    ecs::world w {};
    w.register_component<component::position>();
    w.register_component<component::rotation>();
    w.register_component<component::scale>();
    w.register_component<std::uint32_t>();
    w.register_component<component::large_dummy_struct>(); // testing dat overalignment
    w.register_component<component::even_larger_dummy_struct>();

    auto e1 = w.create_entity();
    assert(w.is_alive(e1));
    assert(e1.version() == 0);
    w.destroy_entity(e1);
    auto e2 = w.create_entity();
    assert(e2.version() == 1); // should be 1 because e1 was recycled

    const component::position cool_pos {.x = 1337.0f, .y = 67.0f, .z = 111.111f};
    const std::uint32_t       cool_num = 67;
    w.add_components(e2, cool_pos, cool_num);
    const auto [x, y] = w.get_components<std::uint32_t, component::position>(e2);
    assert(x == cool_num);
    assert(y == cool_pos);

    w.for_each<std::uint32_t, component::position>([](const std::uint32_t& x, const component::position& y) {
        ENGINE_LOG_DEBUG("holy fuck: {} & {} !!", x, y.z);

        return;
    });

    w.remove_components<std::uint32_t>(e2);

    const auto subscription_id =
        event_bus.subscribe<event::framebuffer_resized>([](const event::framebuffer_resized& e) {
            opengl::set_viewport_dimensions(0, 0, e.width, e.height);
        });

    // Allow client application to initialise

    {
        auto app_logger = std::make_unique<logger>(m_app->get_app_spec().name.c_str());
        m_app->initialise(*this, std::move(app_logger));
    }

    std::chrono::time_point time_last = std::chrono::high_resolution_clock::now();

    while (!m_window->should_close() && !m_app->should_quit()) {
        // Delta time
        const std::chrono::time_point      time_now = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<float> dt_dur   = time_now - time_last;
        time_last                                   = time_now;

        event_bus.poll();

        m_app->pre_render(*this, dt_dur.count());

        m_renderer->render(m_scenes);

        m_app->post_render(*this, dt_dur.count());

        m_window->swap_buffers();
        m_window->poll_events();
    }

    // Notify application we're terminating

    m_app->shutdown(*this);

    // Unsubscribe from events

    event_bus.unsubscribe<event::framebuffer_resized>(subscription_id);

    return std::nullopt;
}
