#pragma once

#include <memory_resource>
#include <new>

namespace kuiper {

namespace ecs {

    class aligned_memory_resource : public std::pmr::memory_resource {
      public:
        aligned_memory_resource() = default;
        aligned_memory_resource(std::size_t alignment)
            : m_alignment(alignment) {}

        virtual ~aligned_memory_resource() = default;

      private:
        virtual void* do_allocate(std::size_t n_bytes, std::size_t alignment) override {
            return ::operator new(n_bytes, std::align_val_t {std::max(alignment, m_alignment)});
        }

        virtual void do_deallocate(void* p, [[maybe_unused]] std::size_t n_bytes, std::size_t alignment) override {
            return ::operator delete(p, std::align_val_t {std::max(alignment, m_alignment)});
        }

        virtual bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
            const auto pmr = dynamic_cast<const aligned_memory_resource*>(&other);
            return (pmr != nullptr) && (pmr->m_alignment == m_alignment);
        }

      private:
        std::size_t m_alignment {alignof(std::max_align_t)};
    };
} // namespace ecs

} // namespace kuiper
