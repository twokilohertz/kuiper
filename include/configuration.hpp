#pragma once

#include "windowing/window.hpp"

#include <cstdint>
#include <optional>
#include <span>

namespace kuiper {

enum class verbosity {
    normal,
    verbose,
    very_verbose
};

class config {
  public:
    static config from_args_and_env(std::span<const char* const> args);

  public:
    std::optional<window::backend> backend {};
    verbosity                      verbosity {verbosity::normal};
};

} // namespace kuiper
