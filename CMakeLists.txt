cmake_minimum_required(VERSION 3.16)
project(AuraSenseRT LANGUAGES CXX)

# Build type and optimization flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strong optimization for this CPU
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Dependencies (OpenCV)
find_package(OpenCV REQUIRED)

# Source files
set(SRC_FILES
    rt_core.cpp
    engine.cpp
    main.cpp
    signature_bank.cpp
    gating_engine.cpp
    failsafe.cpp
    detection_controller.cpp
    crack_statistics.cpp
    yolo_manager.cpp
    yolo_bridge.cpp
    encoder.cpp
    # add any other .cpp files you have
)

add_executable(aura_sense ${SRC_FILES})

target_include_directories(aura_sense PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(aura_sense PRIVATE
    ${OpenCV_LIBS}
    pthread
)

# On Linux, link rt if you use clock_gettime etc.
if(UNIX AND NOT APPLE)
    target_link_libraries(aura_sense PRIVATE rt)
endif()
