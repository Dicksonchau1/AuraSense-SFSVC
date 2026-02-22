# =============================================================================
# CMakeLists.txt — AuraSense SFSVC Production Build System
# =============================================================================

cmake_minimum_required(VERSION 3.16)

# Define the project name and language
project(AuraSenseRT LANGUAGES CXX)

# =============================================================================
# Build Configuration & Compiler Options
# =============================================================================

# Default to a Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Configuring for build type: ${CMAKE_BUILD_TYPE}")

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Compiler-specific flags ---
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Add architecture-specific optimization for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND CMAKE_CXX_FLAGS_RELEASE "-march=native")
        message(STATUS "Release build: enabling -march=native for host optimization.")
    endif()
endif()

# Enable strong compiler warnings for better code quality
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter # Suppress common warning for callback parameters
    )
    message(STATUS "Enabled GCC/Clang compiler warnings.")
elseif(MSVC)
    add_compile_options(/W4)
    message(STATUS "Enabled MSVC compiler warnings.")
endif()

# =============================================================================
# Find Dependencies
# =============================================================================

# Find OpenCV and its required components
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)
if(OpenCV_FOUND)
    message(STATUS "Found OpenCV version: ${OpenCV_VERSION}")
else()
    message(FATAL_ERROR "OpenCV not found. Please install it or set CMAKE_PREFIX_PATH.")
endif()

# Find Threads (required for std::thread)
find_package(Threads REQUIRED)

# =============================================================================
# Source File Organization
# =============================================================================

# Explicitly list all C++ source files for reliability and IDE support.
set(AURA_SOURCES
    src/rt_core.cpp
    src/engine.cpp
    src/encoder.cpp
    src/gating_engine.cpp
    src/signature_bank.cpp
    src/failsafe.cpp
    src/crack_statistics.cpp
    src/yolo_bridge.cpp
    src/detection_controller.cpp
    src/main.cpp
)

message(STATUS "Source files configured:")
foreach(src ${AURA_SOURCES})
    message(STATUS "  ${src}")
endforeach()

# =============================================================================
# Executable Target
# =============================================================================

# Define the executable target
add_executable(aura_sense ${AURA_SOURCES})

# Set include directories for the target
target_include_directories(aura_sense
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src # For local includes like "engine.h"
        ${OpenCV_INCLUDE_DIRS}        # For OpenCV headers
)

# Link required libraries using modern CMake targets
target_link_libraries(aura_sense
    PRIVATE
        OpenCV::opencv_core
        OpenCV::opencv_imgproc
        OpenCV::opencv_imgcodecs
        OpenCV::opencv_highgui
        Threads::Threads
)

# On Linux, link against the 'rt' library for real-time extensions
if(UNIX AND NOT APPLE)
    target_link_libraries(aura_sense PRIVATE rt)
    message(STATUS "Linking with 'rt' library on Linux.")
endif()

# =============================================================================
# Optional: Installation and Packaging
# =============================================================================

install(TARGETS aura_sense
    RUNTIME DESTINATION bin
)

include(CPack)
set(CPACK_PACKAGE_NAME "AuraSenseRT")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AuraSense Real-Time Crack Detection System")
set(CPACK_PACKAGE_VENDOR "AuraSense")