cmake_minimum_required(VERSION 3.16)
project(AuraSenseRT LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

find_package(OpenCV REQUIRED)

set(SRC_FILES
    src/rt_core.cpp
    src/engine.cpp
    src/main.cpp
    src/signature_bank.cpp
    src/gating_engine.cpp
    src/failsafe.cpp
    src/crack_statistics.cpp
    src/yolo_bridge.cpp
    src/encoder.cpp
    src/types.cpp
)

add_executable(aura_sense ${SRC_FILES})

target_include_directories(aura_sense PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(aura_sense PRIVATE
    ${OpenCV_LIBS}
    pthread
)

if(UNIX AND NOT APPLE)
    target_link_libraries(aura_sense PRIVATE rt)
endif()
