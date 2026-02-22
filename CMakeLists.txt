cmake_minimum_required(VERSION 3.16)

project(aurasense_engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)
find_package(pybind11 REQUIRED)

# ------------------------------------------------------------------------------
# Engine Executable
# ------------------------------------------------------------------------------

set(ENGINE_SRC
    src/main.cpp
    src/engine.cpp
    src/encoder.cpp
    src/types.cpp
    src/gating_engine.cpp
    src/rt_core.cpp
)

add_executable(engine ${ENGINE_SRC})

target_include_directories(engine PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(engine PRIVATE
    ${OpenCV_LIBS}
    Boost::boost
    pthread
)

# GCC / Clang optimization flags (Linux)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(engine PRIVATE
        -O3
        -march=native
        -mavx2
        -mfma
        -ffast-math
        -Wno-interference-size
    )
endif()

# ------------------------------------------------------------------------------
# Python Binding
# ------------------------------------------------------------------------------

add_library(rt_core_py MODULE
    src/rt_core_pybind.cpp
    src/rt_core.cpp
)

target_include_directories(rt_core_py PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(rt_core_py PRIVATE
    pybind11::module
    ${OpenCV_LIBS}
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(rt_core_py PRIVATE
        -O3
        -march=native
        -mavx2
        -mfma
        -ffast-math
        -Wno-interference-size
    )
endif()

# On Linux, Python modules use .so
set_target_properties(rt_core_py PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)