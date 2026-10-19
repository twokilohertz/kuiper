#pragma once

#include "component.hpp"
#include "entity.hpp"

#include <algorithm>
#include <iterator>
#include <span>
#include <unordered_map>
#include <vector>

namespace kuiper {

namespace ecs {

    class archetype {
      public:
        using entity_storage       = std::vector<entity_t>;
        using component_vector_map = std::unordered_map<component_id_t, component_vector>;

      public:
        /// @brief Default construct an `archetype`. Use for representing the empty `archetype`.
        archetype() = default;

        /// @brief Construct an `archetype` from pairs of component IDs -> `component_vector`
        archetype(std::span<std::pair<component_id_t, component_vector>> id_vector_pairs)
            : m_vectors(id_vector_pairs.begin(), id_vector_pairs.end()) {}

      public:
        // -- Entities

        void add_entity(entity_t e) {
            m_entities.push_back(e);

            for (auto& [_, vec] : m_vectors) {
                vec.resize(m_entities.size());
            }

            return;
        }

        void remove_entity(entity_t e) {
            const auto it = find_entity(e);
            if (it == m_entities.cend()) {
                return;
            }

            const auto idx = entity_index(e);
            for (auto& [_, vec] : m_vectors) {
                vec.erase(idx);
            }

            m_entities.erase(it);

            return;
        }

        bool contains(entity_t e) const noexcept {
            return find_entity(e) != m_entities.cend();
        }

        entity_storage::const_iterator find_entity(entity_t e) const noexcept {
            return std::ranges::find(m_entities, e);
        }

        std::size_t entity_index(entity_t e) const noexcept {
            return std::ranges::distance(m_entities.cbegin(), find_entity(e));
        }

        // -- Accessors

        const entity_storage& entities() const noexcept {
            return m_entities;
        }

        component_vector_map& component_vectors() noexcept {
            return m_vectors;
        }

        const component_vector_map& component_vectors() const noexcept {
            return m_vectors;
        }

      private:
        entity_storage       m_entities {};
        component_vector_map m_vectors {};
    };

} // namespace ecs

} // namespace kuiper
