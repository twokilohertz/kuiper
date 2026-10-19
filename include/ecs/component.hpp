#pragma once

#include "aligned_memory_resource.hpp"

#include <bit>
#include <bitset>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace kuiper {

namespace ecs {

    // TODO: consider whether these constraints can be tightened to require always non-throwing types
    template<typename T>
    concept decayed_component =
        (std::copyable<T> || std::movable<T>) && std::default_initializable<T> && std::destructible<T>;

    template<typename T> concept component = decayed_component<std::remove_cvref_t<T>>;

#if defined(KUIPER_ECS_MAX_COMPONENTS)
    constexpr std::size_t max_n_components = KUIPER_ECS_MAX_COMPONENTS;
#else
    constexpr std::size_t max_n_components = 64; // arbitrary...
#endif

    using component_mask_t = std::bitset<max_n_components>;
    using component_id_t   = decltype([]() {
        // Select an appropriately sized type to store our component IDs

        constexpr std::size_t n_bits_required = std::bit_width(max_n_components);
        static_assert(n_bits_required <= std::numeric_limits<std::uintmax_t>::digits,
                      "number of bits required to represent all possible components exceeds std::uintmax_t");

        if constexpr (n_bits_required <= std::numeric_limits<std::uint8_t>::digits) {
            return std::uint8_t {};
        } else if constexpr (n_bits_required <= std::numeric_limits<std::uint16_t>::digits) {
            return std::uint16_t {};
        } else if constexpr (n_bits_required <= std::numeric_limits<std::uint32_t>::digits) {
            return std::uint32_t {};
        } else if constexpr (n_bits_required <= std::numeric_limits<std::uint64_t>::digits) {
            return std::uint64_t {};
        } else {
            return std::uintmax_t {};
        }
    }());

    /// Component type information/metadata, used in the type-erased `component_vector`.
    struct component_type_info {
        using constructor_fn      = void (*)(void* const dst);
        using destructor_fn       = void (*)(void* const obj);
        using copy_constructor_fn = void (*)(void* const dst, const void* const src);
        using move_constructor_fn = void (*)(void* const dst, void* const src);

        constructor_fn      constructor;
        destructor_fn       destructor;
        copy_constructor_fn copy_constructor;
        move_constructor_fn move_constructor;

        std::size_t alignment;
        std::size_t component_size;
        std::string type_name;
        bool        is_trivially_copyable; // best case scenario to allow for direct byte-for-byte copies (std::memcpy)
        bool        is_nothrow_move_constructible;
        bool        is_trivially_destructible;
    };

    /// Type-erased component storage (it's basically a std::vector<T> underneath where std::vector<T>::size() %
    /// component_size == 0)
    class component_vector {
      public:
        component_vector(const component_type_info& metadata)
            : m_metadata(metadata), m_aligned_pmr(metadata.alignment), m_data(&m_aligned_pmr) {}
        ~component_vector() {
            clear(); // to handle components that don't have trivial destructors
        }

      public:
        using component_storage = std::pmr::vector<std::byte>;

      public:
        // -- Capacity

        bool empty() const noexcept {
            return m_size == 0;
        }

        std::size_t size() const noexcept {
            return m_size;
        }

        std::size_t capacity() const noexcept {
            return m_data.size() / m_metadata.component_size;
        }

        std::size_t size_bytes() const noexcept {
            return size_to_bytes(size());
        }

        void reserve(std::size_t new_size) {
            return m_data.reserve(size_to_bytes(new_size));
        }

        // -- Modifiers

        void clear() {
            if (!m_metadata.is_trivially_destructible) {
                for (std::size_t i = 0; i < m_size; ++i) {
                    m_metadata.destructor(m_data.data() + size_to_bytes(i));
                }
            }

            m_size = 0;
            return m_data.clear();
        }

        void erase(std::size_t idx) {
            if (idx >= m_size) {
                return;
            }

            const auto start_pos = m_data.cbegin() + idx;
            m_data.erase(start_pos, start_pos + m_metadata.component_size);
            --m_size;

            return;
        }

        /// @brief Push item (copy)
        void push_back(const void* const src) {
            grow_vector(m_size + 1);

            copy_into(src, size() - 1);
            ++m_size;

            return;
        }

        /// @brief Push item (with possible move, see: `should_move` parameter) item
        void push_back(void* const src, bool should_move = false) {
            grow_vector(m_size + 1);

            const auto idx = size() - 1;
            if (m_metadata.is_nothrow_move_constructible && should_move) {
                move_into(src, idx);
            } else {
                copy_into(src, idx);
            }
            ++m_size;

            return;
        }

        /// @brief Copy value into vector at position given by `idx`
        void copy_into(const void* const src, std::size_t idx) {
            if (idx >= size()) {
                return;
            }

            std::byte* const dst = m_data.data() + size_to_bytes(idx);

            if (m_metadata.is_trivially_copyable) {
                std::memcpy(dst, src, m_metadata.component_size);
            } else {
                m_metadata.copy_constructor(dst, src);
            }

            return;
        }

        /// @brief Move value into vector at position given by `idx`
        /// @note If a type is trivially copyable, it is always cheaper to copy.
        /// @note Assumes that `is_nothrow_move_constructible == true`
        void move_into(void* const src, std::size_t idx) {
            if (idx >= size()) {
                return;
            }

            std::byte* const dst = m_data.data() + size_to_bytes(idx);
            m_metadata.move_constructor(dst, src);

            return;
        }

        /// @brief Resize the vector
        /// @note Truncates if `new_size` < `size()`. Default constructs new values of `T`.
        void resize(std::size_t new_size) {
            if (new_size == m_size) {
                return;
            }

            m_data.resize(size_to_bytes(new_size));
            if (new_size > m_size) {
                // Default construct new instances of T

                for (std::size_t offset = m_size; offset < new_size; ++offset) {
                    std::byte* const dst = m_data.data() + size_to_bytes(offset);
                    m_metadata.constructor(dst);
                }
            }

            m_size = new_size;

            return;
        }

        // -- Accessors

        template<component T>
        T* data() noexcept {
            return std::start_lifetime_as<T>(m_data.data());
        }

        template<component T>
        const T* data() const noexcept {
            return std::start_lifetime_as<T>(m_data.data());
        }

        void* value_ptr(std::size_t idx) noexcept {
            return m_data.data() + size_to_bytes(idx);
        }

        const void* value_ptr(std::size_t idx) const noexcept {
            return m_data.data() + size_to_bytes(idx);
        }

        template<component T>
        T& at(std::size_t idx) {
            return data<T>()[idx];
        }

        template<component T>
        const T& at(std::size_t idx) const {
            return data<T>()[idx];
        }

        template<component T>
        T& operator[](std::size_t idx) {
            return data<T>()[idx];
        }

        template<component T>
        const T& operator[](std::size_t idx) const {
            return data<T>()[idx];
        }

      private:
        /// @brief Grows the vector
        /// @note Does not default construct new instances of `T`. Useful for when new elements are going to be
        /// copied/moved into it anyway. See: `resize()`.
        void grow_vector(std::size_t new_size) {
            if (new_size > capacity()) {
                m_data.resize(size_to_bytes(new_size));
            }

            return;
        }

        std::size_t size_to_bytes(std::size_t size) const noexcept {
            return size * m_metadata.component_size;
        }

        std::size_t bytes_to_size(std::size_t bytes) const noexcept {
            return bytes / m_metadata.component_size;
        }

      private:
        /// Component type metadata
        component_type_info m_metadata;
        /// Number of components in this vector
        std::size_t m_size {};
        /// Memory resource for ensuring alignment of data
        aligned_memory_resource m_aligned_pmr;
        /// Type-erased storage for components, cast back to their respective types on demand
        component_storage m_data;
    };

} // namespace ecs

} // namespace kuiper
