#pragma once

#include "error.hpp"
#include "rendering/scene.hpp"

#include "gfx/framebuffer.hpp"

#include <glm/vec4.hpp>

#include <memory>
#include <variant>

namespace kuiper {

class renderer {
  public:
    using s_ptr           = std::shared_ptr<renderer>;
    using renderer_or_err = std::variant<s_ptr, error>;

  public:
    static renderer_or_err initialise();

  public:
    void render(const std::vector<scene>& scenes);

  private:
    opengl::framebuffer m_default_framebuffer {0};
};

} // namespace kuiper
