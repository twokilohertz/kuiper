#pragma once

#include <cstdint>

namespace kuiper {

struct framebuffer_resized_event {
    std::uint32_t width;
    std::uint32_t height;
};

} // namespace kuiper
