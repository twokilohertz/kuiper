#pragma once

#include "key_traits.hpp"

#include <cstddef>
#include <iterator>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace kuiper {

namespace ecs {

    template<typename K>
        requires requires { typename key_traits<K>::id_type; }
    class sparse_set {
      public:
        using traits  = key_traits<K>;
        using id_type = typename traits::id_type;

        using key_type = K;

        using reference       = key_type&;
        using const_reference = const key_type&;
        using pointer         = key_type*;
        using const_pointer   = const key_type*;

        using sparse_container = std::vector<id_type>;
        using dense_container  = std::vector<key_type>;

        using const_iterator         = typename dense_container::const_iterator;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        using sparse_const_span = std::span<const id_type>;
        using dense_const_span  = std::span<const key_type>;

      public:
        // Sentinel value indicating an invalid/freed key
        static constexpr id_type npos = traits::max();

        // Maximum size the key can represent in the sparse container
        constexpr id_type max_size() const noexcept {
            return traits::max() - 1; // see: npos
        }

      public:
        [[nodiscard]]
        bool empty() const noexcept {
            return m_dense.empty();
        }

        [[nodiscard]]
        bool contains(key_type k) const noexcept {
            const auto i = traits::id(k);

            return static_cast<sparse_container::size_type>(i) < m_sparse.size() && m_sparse[i] != npos &&
                   k == m_dense[m_sparse[i]];
        }

        [[nodiscard]]
        std::size_t size() const noexcept {
            return m_dense.size();
        }

        void clear() noexcept {
            m_sparse.clear();
            m_dense.clear();
            return;
        }

        std::pair<const_iterator, bool> insert(key_type k) {
            const auto i = traits::id(k);

            if (i > max_size()) [[unlikely]] {
                throw std::out_of_range("sparse_set: key is out of range of container's max_size()");
            }

            if (contains(k)) {
                return {cend(), false};
            }

            grow_sparse(i + 1); // + 1 as i is an index
            m_dense.push_back(k);
            m_sparse[i] = static_cast<id_type>(m_dense.size() - 1);

            return {cend() - 1, true};
        }

        /// @brief Erase a key from the set
        /// @throw Guaranteed to be noexcept as pop_back() implies an allocation has already previously succeeded :)
        /// @return Iterator to the erased element in the dense container, if the
        const_iterator erase(key_type k) noexcept {
            const auto i = traits::id(k);

            if (!contains(k))
                return cend();

            // It's the ol' swap-and-pop we all know and love

            const auto erase_idx = m_sparse[i];
            auto&      last      = m_dense.back();

            m_dense[erase_idx]         = last;      // rather than swapping we actually just overwrite it here
            m_sparse[traits::id(last)] = erase_idx; // update dense index stored for the swapped key to its new location
            m_dense.pop_back();
            m_sparse[i] = npos;

            return cbegin() + erase_idx;
        }

        // Only the const variants of these are implemented as it would be almost certainly an error to modify the dense
        // array's state and therefore break the invariance & backlinks to the sparse array

        [[nodiscard]]
        const_pointer data() const noexcept {
            return m_dense.data();
        }

        [[nodiscard]]
        const_iterator cbegin() const noexcept {
            return m_dense.cbegin();
        }

        [[nodiscard]]
        const_iterator cend() const noexcept {
            return m_dense.cend();
        }

        [[nodiscard]]
        dense_const_span dense() const noexcept {
            return {m_dense.cbegin(), m_dense.cend()};
        }

      protected: // only a descendant of this class may peer behind the curtain...
        [[nodiscard]]
        sparse_const_span sparse() const noexcept {
            return {m_sparse.cbegin(), m_sparse.cend()};
        }

      private:
        // Grow the sparse array (if required)
        void grow_sparse(std::size_t new_size) {
            if (new_size > m_sparse.size())
                m_sparse.resize(new_size, npos);

            return;
        }

      private:
        sparse_container m_sparse {};
        dense_container  m_dense {};
    };
} // namespace ecs

} // namespace kuiper
