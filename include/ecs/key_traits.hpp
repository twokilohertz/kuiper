#pragma once

#include <concepts>
#include <limits>

namespace kuiper {

namespace ecs {

    // For use with sparse_set (& sparse_map) in requiring either a plain unsigned integer key, or a specialised case
    // for packed entity ID | version unsigned integers (used here for the ECS, see specialisation in entity header)
    // Equally, this enables me to copy-and-paste this sparse_set/map implementation to another project, strip out all
    // of the key_traits stuff and just go back to using the std::unsigned_integral concept if/when I need to! :)

    template<typename K>
    struct key_traits;

    template<std::unsigned_integral K>
    struct key_traits<K> {
        using id_type      = K;
        using version_type = K;

        static constexpr id_type id(K k) noexcept {
            return k;
        }

        static constexpr version_type version(K) noexcept {
            return 0;
        }

        static constexpr id_type max() noexcept {
            return std::numeric_limits<K>::max();
        }
    };
} // namespace ecs

} // namespace kuiper
