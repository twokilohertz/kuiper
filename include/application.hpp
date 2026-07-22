#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace kuiper {

class engine;
class logger;

struct application_spec {
    std::string   name       = "Kuiper Engine Application";
    std::uint32_t width      = 640;
    std::uint32_t height     = 480;
    bool          fullscreen = false;
};

class application {
  public:
    application(const application_spec& app_spec = application_spec())
        : m_app_spec(app_spec) {};
    virtual ~application() = default;

  public:
    using u_ptr = std::unique_ptr<application>;

  public:
    virtual void initialise(kuiper::engine* engine, std::unique_ptr<kuiper::logger> logger) = 0;
    virtual void shutdown(kuiper::engine* engine)                                           = 0;
    virtual bool should_quit()                                                              = 0;
    virtual void pre_render(kuiper::engine* engine, float dt)                               = 0;
    virtual void post_render(kuiper::engine* engine, float dt)                              = 0;

    inline const application_spec& get_app_spec() {
        return m_app_spec;
    }

  protected:
    application_spec m_app_spec = application_spec();
};

/// This function must be implement by the user,
/// defining & creating the application which will be run by the Kuiper engine.
/// Consider this function your application's entrypoint, you can perform
/// initialisation that must occur before your application starts here. NOTE:
/// This function is expected to return a *unique* pointer to the application,
/// to be owned by the engine.
///       Don't store a copy of this pointer to use later as there are no
///       guarantees to whether the pointer is valid.
std::unique_ptr<application> create_application(int argc, char* argv[]);

} // namespace kuiper
