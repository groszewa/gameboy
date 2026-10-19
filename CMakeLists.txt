cmake_minimum_required(VERSION 3.16)

project(gameboy
    VERSION 0.1.0
    LANGUAGES CXX
)

# C++20 for <bit>, std::span, etc. Drop to 17 if you prefer.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For clangd / IDE integration.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sensible warnings; add -Werror if you want it strict.
add_compile_options(-Wall -Wextra -Wpedantic)

option(GAMEBOY_BUILD_TESTS "Build the unit tests" ON)

add_subdirectory(src)

if(GAMEBOY_BUILD_TESTS)
    # doctest: single-header test framework, fetched at configure time.
    # Needs network once; afterwards CMake caches it in the build dir.
    include(FetchContent)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.12
    )
    FetchContent_MakeAvailable(doctest)

    enable_testing()
    add_subdirectory(tests)
endif()
