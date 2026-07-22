#include "resources/image.hpp"

#include <stb_image.h>

#include <cassert>
#include <cstddef>
#include <cstdint>

using namespace kuiper;

image::maybe_image image::from_memory(const std::uint8_t* buf, std::size_t len) {
    int width {0};
    int height {0};
    int n_channels_in_file {0};

    constexpr int n_desired_channels = 4;

    const auto decoded = stbi_load_from_memory(buf, len, &width, &height, &n_channels_in_file, n_desired_channels);
    if (!decoded) {
        return std::nullopt;
    }

    assert(width > 0 && height > 0);

    const std::size_t n_bytes = width * height * n_desired_channels;

    return image {
        std::span {decoded, decoded + n_bytes},
        static_cast<std::uint32_t>(width),
        static_cast<std::uint32_t>(height),
        n_desired_channels
    };
}
