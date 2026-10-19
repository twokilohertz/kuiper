#pragma once

#include "archetype.hpp"
#include "component.hpp"
#include "entity.hpp"
#include "sparse_map.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace kuiper {

namespace ecs {

    class entity_pool {
      public:
        using id_type      = typename entity_t::inner_type;
        using version_type = typename entity_t::inner_type;

        using entity_storage = std::vector<entity_t>;

      public:
        entity_t create() {
            if (m_available > 0) {
                // Entity is available to reuse

                const entity_t     recycled = m_entities[m_next_free];
                const id_type      id       = m_next_free;        // id is the slot index
                const version_type ver      = recycled.version(); // version was bumped on destroy
                m_next_free                 = recycled.id();      // advance the free-list head
                --m_available;

                const entity_t e = entity_t::pack(id, ver);
                m_entities[id]   = e; // bring it back from the dead
                return e;
            } else {
                // Create new entity

                const id_type new_id = static_cast<id_type>(m_entities.size());
                if (new_id > entity_t::max_id()) {
                    throw std::out_of_range("entity_pool: maximum number of entities exceeded");
                }

                const entity_t e = entity_t::pack(new_id, 0);
                m_entities.push_back(e);
                return e;
            }
        }

        void destroy(entity_t e) {
            if (!alive(e)) {
                throw std::invalid_argument("entity_pool: entity is not alive");
            }

            const id_type      id       = e.id();
            const version_type next_ver = static_cast<version_type>(e.version() + 1);

            if (next_ver > entity_t::max_version()) {
                // Version overflowed maximum possible, time for it to rest for good
                m_entities[id] = entity_t::tombstone();
                return;
            }

            m_entities[id] = entity_t::pack(m_next_free, next_ver);
            m_next_free    = id;
            ++m_available;
        }

        bool alive(entity_t e) const noexcept {
            if (e.id() >= m_entities.size()) {
                return false;
            }

            return m_entities[e.id()].version() == e.version();
        }

        std::size_t size() const noexcept {
            return m_entities.size() - m_available;
        }

        bool empty() const noexcept {
            return size() == 0;
        }

        void reserve(std::size_t n) {
            m_entities.reserve(n);
        }

        void clear() {
            m_entities.clear();
            m_next_free = 0;
            m_available = 0;
        }

        const entity_storage& storage() const noexcept {
            return m_entities;
        }

      private:
        entity_storage m_entities {};
        id_type        m_next_free {0};
        std::size_t    m_available {0};
    };

    class world {
      public:
        template<component T>
        void register_component() {
            const auto type_idx = std::type_index(typeid(std::decay_t<T>));
            if (m_component_ids.contains(type_idx)) [[unlikely]] {
                return;
            }

            assert((m_next_component_id != std::numeric_limits<component_id_t>::max()) &&
                   "component IDs will overflow! KUIPER_ECS_MAX_COMPONENTS should be increased.");

            component_id_t new_component_id = m_next_component_id++;
            m_component_ids[type_idx]       = new_component_id;

            // Populate structure with type-specific construction, destruction, etc. methods while we still have the
            // type information

            component_type_info new_component_metadata {
                // -- Constructors, destructors, etc.

                .constructor =
                    [](void* const dst) {
                        std::construct_at(static_cast<T*>(dst));
                    },
                .destructor =
                    [](void* const obj) {
                        std::destroy_at(static_cast<T*>(obj));
                    },
                .copy_constructor =
                    []() {
                        if constexpr (std::is_copy_constructible_v<T>) {
                            return [](void* const dst, const void* const src) {
                                std::construct_at(static_cast<T*>(dst), *static_cast<const T*>(src));
                            };
                        } else {
                            return nullptr; // this shouldn't be called if the type is not copy constructible - but
                                            // it'll be good to segfault early if we do end up in a bad state!
                        }
                    }(),
                .move_constructor =
                    [](void* const dst, void* const src) {
                        std::construct_at(static_cast<T*>(dst), std::move(*static_cast<T*>(src)));
                    },

                // -- Metadata

                .alignment                     = alignof(T),
                .component_size                = sizeof(T),
                .type_name                     = typeid(T).name(),
                .is_trivially_copyable         = std::is_trivially_copyable_v<T>,
                .is_nothrow_move_constructible = std::is_nothrow_move_constructible_v<T>,
                .is_trivially_destructible     = std::is_trivially_destructible_v<T>};

            m_component_metadata[new_component_id] = new_component_metadata; // FIXME: this might be useless
            m_component_vectors.insert_or_assign(new_component_id, new_component_metadata);

            return;
        }

        // -- Entities

        entity_t create_entity() {
            entity_t   new_entity     = m_entities.create();
            archetype& null_archetype = get_archetype(component_mask_t {});
            null_archetype.add_entity(new_entity);
            m_entity_archetypes.insert(new_entity,
                                       &null_archetype); // TODO: could we use nullptr instead of The Empty Archetype?

            return new_entity;
        }

        void destroy_entity(entity_t e) {
            if (!is_alive(e)) {
                return;
            }

            m_entity_archetypes[e]->remove_entity(e);

            return m_entities.destroy(e);
        }

        template<component... Components>
        void add_components(entity_t e, Components&&... component) {
            if (!is_alive(e)) {
                return;
            }

            archetype* current_archetype = m_entity_archetypes[e];

            const auto it = std::ranges::find_if(m_archetypes, [&current_archetype](const auto& entry) {
                return current_archetype == &entry.second;
            });

            if (it == m_archetypes.cend()) {
                return;
            }

            const component_mask_t current_mask = it->first;
            const component_mask_t new_mask     = component_mask<Components...>();
            const component_mask_t target_mask  = current_mask | new_mask;

            if (current_mask == target_mask) {
                return;
            }

            archetype& target_archetype = get_archetype(target_mask);
            target_archetype.add_entity(e);

            // Copy old components over

            const auto entity_idx = current_archetype->entity_index(e);
            for (const auto& [id, src_vec] : current_archetype->component_vectors()) {
                auto& dst_vec = target_archetype.component_vectors().at(id);
                dst_vec.copy_into(src_vec.value_ptr(entity_idx), dst_vec.size() - 1);
            }

            // Copy new components

            (
                [&]() {
                    using ComponentType     = std::decay_t<Components>;
                    const component_id_t id = component_id<ComponentType>();

                    auto& dst_vec = target_archetype.component_vectors().at(id);
                    dst_vec.copy_into(&component, dst_vec.size() - 1);
                }(),
                ...);

            current_archetype->remove_entity(e);
            m_entity_archetypes[e] = &target_archetype;

            return;
        }

        bool is_alive(entity_t e) {
            return m_entities.alive(e);
        }

        // -- Component accessors

        template<component Component>
        Component& get(entity_t e) {
            if (!is_alive(e)) {
                throw nullptr; // FIXME: this sucks
            }

            archetype* entity_archetype = m_entity_archetypes[e];
            const auto idx              = entity_archetype->entity_index(e);
            return entity_archetype->component_vectors().at(component_id<Component>()).template at<Component>(idx);
        }

      private:
        // -- Components

        template<component T>
        component_id_t component_id() {
            const auto type_idx = std::type_index(typeid(std::decay_t<T>));
            assert(m_component_ids.contains(type_idx) &&
                   "component has not been registered before calling component_id()!");

            return m_component_ids[type_idx];
        }

        template<component T>
        const component_type_info& component_metadata() {
            return component_metadata(component_id<T>());
        }

        const component_type_info& component_metadata(component_id_t id) {
            return m_component_metadata[id];
        }

        template<component... Components>
        component_mask_t component_mask() {
            component_mask_t mask {};
            (mask.set(component_id<Components>()), ...);
            return mask;
        }

      private:
        // -- Archetypes

        archetype& get_archetype(const component_mask_t& mask) {
            if (m_archetypes.contains(mask)) [[likely]] {
                return m_archetypes.at(mask);
            } else {
                // Create new archetype

                // We actually want a std::array here, really. However, component_vector isn't default constructible.
                // Alternatively: std::inplace_vector! (in C++26... :/)
                std::vector<std::pair<component_id_t, component_vector>> id_vector_pairs;
                id_vector_pairs.reserve(mask.size());

                for (component_id_t i = 0; i < mask.size(); ++i) {
                    if (!mask.test(i))
                        continue;

                    id_vector_pairs.emplace_back(i, m_component_vectors.at(i));
                }

                m_archetypes.insert(std::make_pair(mask, archetype(std::span {id_vector_pairs})));

                return m_archetypes.at(mask);
            }
        }

      private:
        // -- Entities

        entity_pool                      m_entities {};
        sparse_map<entity_t, archetype*> m_entity_archetypes {};

        // -- Components

        component_id_t m_next_component_id {0};
        // TODO: could we use std::flat_map here?
        std::unordered_map<std::type_index, component_id_t>     m_component_ids {};
        std::unordered_map<component_id_t, component_type_info> m_component_metadata {};
        std::unordered_map<component_id_t, component_vector>    m_component_vectors {};

        // -- Archetypes

        std::unordered_map<component_mask_t, archetype> m_archetypes {};
    };

} // namespace ecs

} // namespace kuiper
