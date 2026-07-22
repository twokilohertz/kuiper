#include "rendering/renderer.hpp"

#include "gfx/opengl.hpp"

using namespace kuiper;

renderer::renderer_or_err renderer::initialise() {
    auto renderer = std::make_shared<kuiper::renderer>();

    opengl::set_clear_colour(0.333f, 0.333f, 0.333f, 0.333f);

    return renderer;
}

void renderer::render(const std::vector<scene>& scenes) {
    // Ensure framebuffer is bound

    if (scenes.empty()) {
        m_default_framebuffer.bind(GL_DRAW_FRAMEBUFFER);
        opengl::set_clear_colour(0.333f, 0.333f, 0.333f, 0.333f);
        opengl::clear_framebuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    return;
}
