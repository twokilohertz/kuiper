#pragma once

#include "key_traits.hpp"

#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

namespace kuiper {

namespace ecs {

    template<std::unsigned_integral T, std::size_t VersionBits = ((sizeof(T) * CHAR_BIT) / 2)>
        requires(VersionBits < sizeof(T) * CHAR_BIT)
    class entity {
      public:
        using inner_type = T;

        static constexpr std::size_t version_bits = VersionBits;
        static constexpr std::size_t id_bits      = std::numeric_limits<T>::digits - version_bits;

      public:
        /// @brief Maximum possible value the version can contain
        static constexpr std::size_t max_version() noexcept {
            return (std::size_t {1} << version_bits) - 1;
        }

        /// @brief Maximum possible value the ID can contain
        static constexpr std::size_t max_id() noexcept {
            return (std::size_t {1} << id_bits) - 1;
        }

      public:
        /// @brief Construct an entity from an ID & version
        static constexpr entity pack(T id, T version) noexcept {
            return {(version << id_bits) | id};
        }

        /// @brief An invalid entity
        static constexpr entity null() noexcept {
            return pack(max_id(), 0);
        }

        /// @brief A entity that is well and truly dead forever (retired)
        static constexpr entity tombstone() noexcept {
            return pack(0, max_version());
        }

        /// @brief Returns the next generation of a given entity, or `std::nullopt` on overflow
        static constexpr std::optional<entity> next_version(entity e) noexcept {
            const auto v = e.version();

            if (v == max_version()) {
                return std::nullopt;
            }

            return pack(e.id(), v + 1);
        }

      public:
        /// @brief Returns a pair of the entity's unpacked [ID, version]
        std::pair<T, T> unpack() const noexcept {
            return {id(), version()};
        }

        /// @brief Get the version (generation) of this entity
        T version() const noexcept {
            return inner >> id_bits;
        }

        /// @brief Get the ID of this entity
        T id() const noexcept {
            return (inner << version_bits) >> version_bits;
        }

        constexpr bool is_null() const noexcept {
            return *this == null();
        }

        constexpr bool is_tombstone() const noexcept {
            return *this == tombstone();
        }

        constexpr bool operator==(const entity& rhs) const noexcept {
            return inner == rhs.inner;
        }

        constexpr bool operator!=(const entity& rhs) const noexcept {
            return !(*this == rhs);
        }

      public:
        T inner;
    };

    // A 32-bit unsigned integer with 12 version bits gives us just over a million possible entity IDs, and 4096
    // generations. Will review this if/when we run out of IDs or versions :^)
    using entity_t = entity<std::uint32_t, 12>;

    template<std::unsigned_integral T, std::size_t VersionBits>
    struct key_traits<entity<T, VersionBits>> {
        using id_type      = T;
        using version_type = T;

        static constexpr T id(entity<T, VersionBits> e) noexcept {
            return e.id();
        }

        static constexpr T version(entity<T, VersionBits> e) noexcept {
            return e.version();
        }

        static constexpr T max() noexcept {
            return entity<T, VersionBits>::max_id();
        }
    };

} // namespace ecs

} // namespace kuiper
