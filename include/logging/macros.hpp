#pragma once

#include "logging/engine_logger.hpp"

namespace kuiper {

#if !defined(NDEBUG)
#define ENGINE_LOG_DEBUG(args...) engine_logger::get().debug(args);
#else
#define ENGINE_LOG_DEBUG(args...)
#endif
} // namespace kuiper
