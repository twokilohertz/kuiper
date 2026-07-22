cmake_minimum_required(VERSION 3.31)

project(stb
        VERSION 1.0
        DESCRIPTION "stb single-file public domain libraries for C/C++"
        HOMEPAGE_URL https://github.com/nothings/stb
        LANGUAGES C
)

add_library(
    stb
    src/stb_image.c
)

target_include_directories(stb PUBLIC include)
