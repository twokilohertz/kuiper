#include "entrypoint.hpp"
#include "application.hpp"
#include "configuration.hpp"
#include "engine.hpp"
#include "error.hpp"
#include "logging/engine_logger.hpp"
#include "logging/logger.hpp"

#include <cstdlib>
#include <exception>
#include <memory>
#include <utility>

// Implemented by the user
extern std::unique_ptr<kuiper::application> kuiper::create_application(int argc, char* argv[]);

int main(int argc, char* argv[]) {
    return kuiper::entrypoint(argc, argv);
}

int kuiper::entrypoint(int argc, char* argv[]) {
    // Initialise logging

    auto& logger = engine_logger::get();

    // Parse command line arguments

    const auto config = config::from_args_and_env({argv, argv + static_cast<std::size_t>(argc)});

    switch (config.verbosity) {
    case verbosity::normal:
        logger.set_level(logger::default_level);
        break;
    case verbosity::verbose:
        logger.set_level(logger::level::debug);
        break;
    case verbosity::very_verbose:
        logger.set_level(logger::level::trace);
        break;
    default:
        break;
    }

    // Create instance of client application

    std::unique_ptr<kuiper::application> app = kuiper::create_application(argc, argv);

    if (app == nullptr) {
        logger.critical("Client application was nullptr");
        return EXIT_FAILURE;
    }

    // Initialise engine & begin execution

    try {
        auto maybe_engine = engine::initialise(std::move(app), config);
        if (const auto* err = std::get_if<error>(&maybe_engine)) {
            logger.critical("Failed to initialise engine: {}", err->description);
            return EXIT_FAILURE;
        }

        auto engine = std::get<kuiper::engine>(std::move(maybe_engine));
        engine.run();

    } catch (const std::exception& ex) {
        logger.critical("Unhandled std::exception: {}", ex.what());
        return EXIT_FAILURE;
    } catch (...) {
        logger.critical("Unknown unhandled exception!");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS; // TODO: Return error exit code if the app encounters an error during execution
}
