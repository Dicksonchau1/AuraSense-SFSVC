cmake_minimum_required(VERSION 3.16)

project(aurasense_engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED)

# ==============================================================================
# Engine
# ==============================================================================

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

# If Boost is installed via MSYS2 and header-only System is used,
# you don't need to link boost_system explicitly.
# Beast/Asio are header-only; only platform libs are needed.
target_link_libraries(engine PRIVATE
    ${OpenCV_LIBS}
    ws2_32          # Windows sockets
    wsock32         # (often optional but safe)
)

if (WIN32)
    target_link_libraries(engine PRIVATE
        kernel32 user32 gdi32 winspool shell32
        ole32 oleaut32 uuid comdlg32 advapi32
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(engine PRIVATE
        -O3 -mavx2 -mfma -ffast-math -Wno-interference-size
    )
endif()

# ==============================================================================
# Python Binding
# ==============================================================================

find_package(pybind11 REQUIRED)

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

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(rt_core_py PRIVATE
        -O3 -mavx2 -mfma -ffast-math -Wno-interference-size
    )
endif()

set_target_properties(rt_core_py PROPERTIES
    PREFIX ""
    SUFFIX ".pyd"
)
