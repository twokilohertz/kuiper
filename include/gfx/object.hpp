#pragma once

#include <cstdint>

namespace kuiper {

namespace opengl {
    class object {
      public:
        object(std::uint32_t name)
            : m_name(name) {}

      public:
        using delete_fn = void (*)(std::int32_t, const std::uint32_t*);
        using bind_fn   = void (*)(std::uint32_t, std::uint32_t);

      public:
        std::uint32_t name() {
            return m_name;
        }

        void destroy(delete_fn fn) {
            const auto n = name();
            fn(1, &n);
            m_name = 0;
            return;
        }

        void bind(bind_fn fn, std::uint32_t target) {
            return fn(target, name());
        }

      private:
        std::uint32_t m_name {0};
    };

}; // namespace opengl

} // namespace kuiper
