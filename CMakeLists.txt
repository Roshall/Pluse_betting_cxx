cmake_minimum_required(VERSION 3.20)
project(betting_by_time VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# Debug mode: enable sanitizers
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()

# Fetch external dependencies
include(FetchContent)

# Fetch Catch2 for testing
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.15.0
)
FetchContent_MakeAvailable(Catch2)

# Fetch Eigen for linear algebra operations
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 5.0.0
)
FetchContent_MakeAvailable(eigen)

# Create main library target
add_library(betting_by_time STATIC
    src/utilities.cpp
    src/geo_checking.cpp
    src/sequence_checking.cpp
    src/bet_once.cpp
    src/vanilla_betting.cpp
    src/adaptive_betting.cpp
    src/framework.cpp
)

target_include_directories(betting_by_time PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(betting_by_time PUBLIC
    Eigen3::Eigen
)

# Release mode optimizations
target_compile_options(betting_by_time PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native>
    $<$<CONFIG:RelWithDebInfo>:-O2 -g>
)

# Define EIGEN_NO_DEBUG for release builds
target_compile_definitions(betting_by_time PRIVATE
    $<$<CONFIG:Release>:EIGEN_NO_DEBUG>
)

# Add subdirectories
add_subdirectory(tests)
add_subdirectory(benchmarks)
add_subdirectory(examples)
