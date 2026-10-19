#pragma once

#include <cstdint>

namespace kuiper {

namespace event {

    struct framebuffer_resized {
        std::uint32_t width;
        std::uint32_t height;
    };

} // namespace event

} // namespace kuiper
