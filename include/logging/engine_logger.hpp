#pragma once

#include "logging/logger.hpp"

namespace kuiper {

/// @brief Singleton class providing engine-wide logging facilities
class engine_logger : public logger {
  private:
    engine_logger()
        : logger("engine") {}

  public:
    ~engine_logger() = default;

  public:
    static inline engine_logger& get(void) {
        static engine_logger s_instance;
        return s_instance;
    }
};

} // namespace kuiper
