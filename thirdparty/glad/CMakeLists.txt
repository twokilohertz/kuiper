cmake_minimum_required(VERSION 3.31)

project(glad
        VERSION 1.0.0
        DESCRIPTION "Multi-Language Vulkan/GL/GLES/EGL/GLX/WGL Loader-Generator based on the official specs"
        HOMEPAGE_URL https://github.com/dav1dde/glad
        LANGUAGES C
)

add_library(glad src/gl.c)

target_include_directories(glad PUBLIC include)
