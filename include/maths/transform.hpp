#pragma once

namespace kuiper {

namespace component {

    /// Three-dimensional position
    struct position {
        float x {0.0f};
        float y {0.0f};
        float z {0.0f};

        constexpr bool operator==(const position&) const noexcept = default;
    };

    /// Euler angle rotation (pitch, yaw & roll)
    struct rotation {
        float x {0.0f};
        float y {0.0f};
        float z {0.0f};

        constexpr bool operator==(const rotation&) const noexcept = default;
    };

    /// Scaling factor per axis
    struct scale {
        float x {0.0f};
        float y {0.0f};
        float z {0.0f};

        constexpr bool operator==(const scale&) const noexcept = default;
    };

    struct alignas(32) large_dummy_struct {
        float xs[8];
    };

    struct alignas(64) even_larger_dummy_struct {
        float xs[16];
    };

} // namespace component

} // namespace kuiper
