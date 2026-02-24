# =============================================================================
# CMakeLists.txt — AuraSense SFSVC + Failsafe Integrated Build
# =============================================================================

cmake_minimum_required(VERSION 3.16)

project(AuraSenseRT LANGUAGES CXX)

# =============================================================================
# Build Configuration & Compiler Options
# =============================================================================

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Configuring for build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG   "-g -O0 -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND CMAKE_CXX_FLAGS_RELEASE "-march=native" "-ffast-math")
        message(STATUS "Release build: enabling -march=native -ffast-math.")
    endif()
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
    )
    message(STATUS "Enabled GCC/Clang compiler warnings.")
elseif(MSVC)
    add_compile_options(/W4)
    message(STATUS "Enabled MSVC compiler warnings.")
endif()

# =============================================================================
# Dependencies
# =============================================================================

find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)
if(OpenCV_FOUND)
    message(STATUS "Found OpenCV version: ${OpenCV_VERSION}")
else()
    message(FATAL_ERROR "OpenCV not found. Please install it or set CMAKE_PREFIX_PATH.")
endif()

find_package(Threads REQUIRED)

# =============================================================================
# Failsafe / Policy / Middleware Libraries
# =============================================================================

# Failsafe core
add_library(failsafe STATIC
    src/failsafe.cpp
)

target_include_directories(failsafe PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Degraded mode policy
add_library(degraded_mode_policy STATIC
    src/degraded_mode_policy.cpp
)

target_include_directories(degraded_mode_policy PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(degraded_mode_policy
    PUBLIC
        failsafe
)

# Drone middleware
add_library(drone_middleware STATIC
    src/drone_middleware.cpp
)

target_include_directories(drone_middleware PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(drone_middleware
    PUBLIC
        failsafe
        degraded_mode_policy
        Threads::Threads
)

# =============================================================================
# Core AuraSense Multi-Lane Engine
# =============================================================================

set(AURA_SOURCES
    src/rt_core.cpp
    src/engine.cpp
    src/encoder.cpp
    src/gating_engine.cpp
    src/signature_bank.cpp
    src/crack_statistics.cpp
    src/yolo_bridge.cpp
    src/detection_controller.cpp
    src/main.cpp
)

add_executable(aura_sense ${AURA_SOURCES})

target_include_directories(aura_sense
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(aura_sense
    PRIVATE
        OpenCV::opencv_core
        OpenCV::opencv_imgproc
        OpenCV::opencv_imgcodecs
        OpenCV::opencv_highgui
        Threads::Threads
        drone_middleware
        degraded_mode_policy
        failsafe
)

if(UNIX AND NOT APPLE)
    target_link_libraries(aura_sense PRIVATE rt)
    message(STATUS "Linking with 'rt' on Linux.")
endif()

# =============================================================================
# Optional: Failsafe Demo Executable
# =============================================================================

add_executable(failsafe_demo
    src/failsafe_example.cpp
)

target_include_directories(failsafe_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(failsafe_demo
    PRIVATE
        drone_middleware
        degraded_mode_policy
        failsafe
        Threads::Threads
)

# =============================================================================
# Installation
# =============================================================================

install(TARGETS aura_sense failsafe degraded_mode_policy drone_middleware failsafe_demo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)

# =============================================================================
# Testing (optional)
# =============================================================================

enable_testing()

add_test(NAME failsafe_demo_test
    COMMAND failsafe_demo
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# =============================================================================
# Build Info
# =============================================================================

message(STATUS "")
message(STATUS "=============================================================")
message(STATUS "AuraSense RT + Failsafe - Build Configuration")
message(STATUS "=============================================================")
message(STATUS "Build Type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler:      ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard:      ${CMAKE_CXX_STANDARD}")
message(STATUS "Install Prefix:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Build Instructions:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake .. -DCMAKE_BUILD_TYPE=Release")
message(STATUS "  make -j\$(nproc)")
message(STATUS "  ./aura_sense")
message(STATUS "=============================================================")
message(STATUS "")