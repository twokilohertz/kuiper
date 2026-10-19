cmake_minimum_required(VERSION 3.31)

project(kuiper
        VERSION 0.1.0
        DESCRIPTION "Real-time rendering engine using C++ & OpenGL 4.6"
        LANGUAGES CXX
)

add_library(
    kuiper
    src/entrypoint.cpp
    src/engine.cpp
    src/entity/ecs.cpp
    src/gfx/opengl.cpp
    src/gfx/utils.cpp
    src/window/glfw.cpp
    src/rendering/renderer.cpp
    src/resources/image.cpp
    src/configuration.cpp
)

# C++ standard & compiler extensions
target_compile_features(kuiper PRIVATE cxx_std_23)
set_target_properties(kuiper PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Warnings

if (MSVC)
    target_compile_options(kuiper PRIVATE /W4)
else()
    target_compile_options(kuiper PRIVATE -Wall -Wextra -Werror)
endif()

target_include_directories(kuiper PUBLIC include)

# Dependencies

add_subdirectory(thirdparty/glad)
add_subdirectory(thirdparty/stb)

include(FetchContent)

# spdlog
set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE_HO OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS_HO OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_DISABLE_DEFAULT_LOGGER ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(spdlog)

# GLFW

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
target_compile_definitions(kuiper PRIVATE GLFW_INCLUDE_NONE=1)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.5.1
)
FetchContent_MakeAvailable(glfw)

# GLM

set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

target_link_libraries(kuiper PUBLIC spdlog glfw glad glm stb)
