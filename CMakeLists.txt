# =============================================================================
# CMakeLists.txt — AuraSense SFSVC Production Build System
# =============================================================================

cmake_minimum_required(VERSION 3.16)
project(AuraSenseRT LANGUAGES CXX)

# =============================================================================
# Build Configuration & Compiler Options
# =============================================================================

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific flags
# Use generator expressions for per-configuration flags
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

# Find OpenCV
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)
if(OpenCV_FOUND)
    message(STATUS "Found OpenCV version: ${OpenCV_VERSION}")
    message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "OpenCV not found. Please install it or set CMAKE_PREFIX_PATH.")
endif()

# Find Threads (required for std::thread)
find_package(Threads REQUIRED)


# =============================================================================
# Source File Organization
# =============================================================================

# Automatically collect all .cpp files in the 'src' directory
# This is more maintainable than a static list
file(GLOB_RECURSE AURA_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

message(STATUS "Source files found:")
foreach(src ${AURA_SOURCES})
    message(STATUS "  ${src}")
endforeach()


# =============================================================================
# Executable Target
# =============================================================================

add_executable(aura_sense ${AURA_SOURCES})

# Set the include directories for the target
# PRIVATE means these directories are only used when compiling aura_sense itself
target_include_directories(aura_sense
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src # For local includes like "engine.h"
        ${OpenCV_INCLUDE_DIRS}        # For OpenCV headers
)

# Link libraries to the executable
# PRIVATE means these libraries are only needed by aura_sense
target_link_libraries(aura_sense
    PRIVATE
        ${OpenCV_LIBS} # All OpenCV libraries
        Threads::Threads # CMake's standard way to link pthreads
)

# On Linux, link against the 'rt' library for real-time extensions (clock_gettime)
if(UNIX AND NOT APPLE)
    target_link_libraries(aura_sense PRIVATE rt)
    message(STATUS "Linking with 'rt' library on Linux.")
endif()


# =============================================================================
# Optional: Installation and Packaging
# =============================================================================

# Define where to install the binary
install(TARGETS aura_sense
    RUNTIME DESTINATION bin
)

# Optional: Create a package (e.g., a .tar.gz file)
include(CPack)
set(CPACK_PACKAGE_NAME "AuraSenseRT")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AuraSense Real-Time Crack Detection System")
set(CPACK_PACKAGE_VENDOR "AuraSense")