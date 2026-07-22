#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace kuiper {

class image {
  private:
    image(std::span<const std::uint8_t> data, std::uint32_t width, std::uint32_t height, std::uint32_t channels)
        : m_data(data.begin(), data.end()), m_width(width), m_height(height), m_channels(channels) {}

  public:
    using maybe_image = std::optional<image>;

  public:
    static maybe_image from_memory(const std::uint8_t* buf, std::size_t len);

  public:
    const std::vector<std::uint8_t>& data() const {
        return m_data;
    }

    std::uint32_t width() const {
        return m_width;
    }

    std::uint32_t height() const {
        return m_height;
    }

    std::uint32_t channels() const {
        return m_channels;
    }

  private:
    std::vector<std::uint8_t> m_data {};
    std::uint32_t             m_width {0};
    std::uint32_t             m_height {0};
    std::uint32_t             m_channels {0};
};

} // namespace kuiper
