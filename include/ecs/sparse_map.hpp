#pragma once

#include "sparse_set.hpp"

#include <iterator>
#include <ranges>
#include <utility>

namespace kuiper {

namespace ecs {

    template<typename K, typename V>
    class sparse_map : public sparse_set<K> {
      public:
        using key_type    = typename sparse_set<K>::key_type;
        using mapped_type = V;
        using value_type  = std::pair<key_type, mapped_type>;

        using reference       = mapped_type&;
        using const_reference = const mapped_type&;
        using pointer         = mapped_type*;
        using const_pointer   = const mapped_type*;

        using mapped_container = std::vector<mapped_type>;

        using span       = std::span<mapped_type>;
        using const_span = std::span<const mapped_type>;

        using zip_type =
            decltype(std::views::zip(std::declval<typename sparse_set<K>::dense_const_span>(), std::declval<span>()));
        using iterator = std::ranges::iterator_t<zip_type>;
        using sentinel = std::ranges::sentinel_t<zip_type>;

        using const_zip_type = decltype(std::views::zip(std::declval<typename sparse_set<K>::dense_const_span>(),
                                                        std::declval<const_span>()));
        using const_iterator = std::ranges::iterator_t<const_zip_type>;
        using const_sentinel = std::ranges::sentinel_t<const_zip_type>;

      public:
        [[nodiscard]]
        bool empty() const noexcept {
            return sparse_set<K>::empty();
        }

        [[nodiscard]]
        bool contains(key_type k) const noexcept {
            return sparse_set<K>::contains(k);
        }

        [[nodiscard]]
        std::size_t size() const noexcept {
            return sparse_set<K>::size();
        }

        void clear() noexcept {
            sparse_set<K>::clear();
            m_values.clear();

            return;
        }

        std::pair<iterator, bool> insert(key_type k, const mapped_type& v) {
            return emplace(k, v);
        }

        std::pair<iterator, bool> insert(key_type k, mapped_type&& v) {
            return emplace(k, std::move(v));
        }

        template<class... Args>
        std::pair<iterator, bool> emplace(key_type k, Args&&... args) {
            const auto& [it, inserted] = sparse_set<K>::insert(k);

            if (!inserted)
                return {end(), false};

            m_values.emplace_back(std::forward<Args>(args)...);

            const auto insert_idx = std::distance(sparse_set<K>::cbegin(), it);

            return {begin() + insert_idx, true};
        }

        std::pair<iterator, bool> erase(key_type k) noexcept {
            const auto it = sparse_set<K>::erase(k);

            // We check for !empty() here as end() can become ambiguous in the case where the
            // final element has been erased from the set but said final element still needs to
            // be erased from the map too

            // TODO: can we make this less ambiguous & error prone by using a std::optional or something?

            if (it == sparse_set<K>::cend() && !sparse_set<K>::empty())
                return {end(), false};

            const auto erase_idx = std::distance(sparse_set<K>::cbegin(), it);
            m_values[erase_idx]  = m_values.back();
            m_values.pop_back();

            return {begin() + erase_idx, true};
        }

        [[nodiscard]]
        pointer data() noexcept {
            return m_values.data();
        }

        [[nodiscard]]
        const_pointer data() const noexcept {
            return m_values.data();
        }

        [[nodiscard]]
        iterator begin() noexcept {
            return std::views::zip(keys(), values()).begin();
        }

        [[nodiscard]]
        sentinel end() noexcept {
            return std::views::zip(keys(), values()).end();
        }

        [[nodiscard]]
        const_iterator cbegin() const noexcept {
            return std::views::zip(keys(), values()).begin();
        }

        [[nodiscard]]
        const_sentinel cend() const noexcept {
            return std::views::zip(keys(), values()).end();
        }

        [[nodiscard]]
        sparse_set<K>::dense_const_span keys() const noexcept {
            return sparse_set<K>::dense();
        }

        [[nodiscard]]
        span values() noexcept {
            return {m_values.begin(), m_values.end()};
        }

        [[nodiscard]]
        const_span values() const noexcept {
            return {m_values.cbegin(), m_values.cend()};
        }

        [[nodiscard]]
        reference operator[](key_type k) {
            return m_values[sparse_set<K>::sparse()[k.id()]];
        }

        [[nodiscard]]
        const_reference operator[](key_type k) const {
            return m_values[sparse_set<K>::sparse()[k.id()]];
        }

      private:
        mapped_container m_values {};
    };

} // namespace ecs

} // namespace kuiper
