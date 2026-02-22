cmake_minimum_required(VERSION 3.16)

project(aurasense_engine LANGUAGES C CXX)

# ------------------------------------------------------------------------------
# Global Configuration
# ------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ------------------------------------------------------------------------------
# Build Options
# ------------------------------------------------------------------------------

option(ENABLE_AVX2 "Enable AVX2 optimizations" ON)
option(ENABLE_FAST_MATH "Enable -ffast-math (unsafe FP optimizations)" OFF)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)
find_package(pybind11 REQUIRED)
find_package(Threads REQUIRED)

# ------------------------------------------------------------------------------
# Compiler Flags (GNU / Clang)
# ------------------------------------------------------------------------------

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

    set(COMMON_OPT_FLAGS
        -O3
        -Wall
        -Wextra
        -Wpedantic
        -Wno-interference-size
    )

    if (ENABLE_AVX2)
        list(APPEND COMMON_OPT_FLAGS
            -march=native
            -mavx2
            -mfma
        )
    endif()

    if (ENABLE_FAST_MATH)
        list(APPEND COMMON_OPT_FLAGS -ffast-math)
    endif()

endif()

# ------------------------------------------------------------------------------
# Engine Core Sources (Shared)
# ------------------------------------------------------------------------------

set(ENGINE_CORE_SRC
    src/rt_core.cpp
    src/gating_engine.cpp
    src/types.cpp
)

# ------------------------------------------------------------------------------
# Engine Application Sources
# ------------------------------------------------------------------------------

set(ENGINE_APP_SRC
    src/main.cpp
    src/engine.cpp
    src/encoder.cpp
    src/async_camera.cpp
)

# ------------------------------------------------------------------------------
# Engine Executable
# ------------------------------------------------------------------------------

add_executable(engine
    ${ENGINE_CORE_SRC}
    ${ENGINE_APP_SRC}
)

target_include_directories(engine PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(engine PRIVATE
    ${OpenCV_LIBS}
    Boost::boost
    Threads::Threads
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(engine PRIVATE ${COMMON_OPT_FLAGS})
endif()

# ------------------------------------------------------------------------------
# Python Binding (rt_core only)
# ------------------------------------------------------------------------------

add_library(rt_core_py MODULE
    src/rt_core_pybind.cpp
    ${ENGINE_CORE_SRC}
)

target_include_directories(rt_core_py PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(rt_core_py PRIVATE
    pybind11::module
    Threads::Threads
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(rt_core_py PRIVATE ${COMMON_OPT_FLAGS})
endif()

# Python module properties (Linux)
set_target_properties(rt_core_py PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

# ------------------------------------------------------------------------------
# Install Rules (Optional)
# ------------------------------------------------------------------------------

install(TARGETS engine
        RUNTIME DESTINATION bin)

install(TARGETS rt_core_py
        LIBRARY DESTINATION lib)