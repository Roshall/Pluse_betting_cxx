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

# Fetch external dependencies (only if not already available from parent)
include(FetchContent)

# Fetch Catch2 for testing (only if not already fetched by parent)
if(NOT TARGET Catch2::Catch2WithMain)
    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.15.0
    )
    FetchContent_MakeAvailable(Catch2)
endif()

# Fetch Eigen for linear algebra operations (only if not already fetched by parent)
if(NOT TARGET Eigen3::Eigen)
    FetchContent_Declare(
      eigen
      GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
      GIT_TAG 5.0.0
    )
    FetchContent_MakeAvailable(eigen)
endif()

# Library type configuration
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Create main library target
add_library(betting_by_time
    src/utilities.cpp
    include/betting_by_time/capital/geo_checking.hpp
    include/betting_by_time/capital/sequence_checking.hpp
    include/betting_by_time/bet_once.hpp
    include/betting_by_time/strategies/vanilla_betting.hpp
    include/betting_by_time/strategies/adaptive_betting.hpp
    src/framework.cpp
)

target_include_directories(betting_by_time PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(betting_by_time PUBLIC
    Eigen3::Eigen
)

# Release mode optimizations - PUBLIC to ensure consistent template instantiation
target_compile_options(betting_by_time PUBLIC
    $<$<CONFIG:Release>:-O3 -march=native>
    $<$<CONFIG:RelWithDebInfo>:-O2 -g>
)

# Define EIGEN_NO_DEBUG for release builds - PUBLIC to avoid ODR violations
target_compile_definitions(betting_by_time PUBLIC
    $<$<CONFIG:Release>:EIGEN_NO_DEBUG>
)

# Add subdirectories
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation configuration
# Skip installation when building from root selva project (SELVA_ROOT_BUILD is set)
if(NOT DEFINED SELVA_ROOT_BUILD)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # Install the library
    install(TARGETS betting_by_time
        EXPORT betting_by_timeTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    # Install header files
    install(DIRECTORY include/betting_by_time/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/betting_by_time
        FILES_MATCHING PATTERN "*.hpp"
    )

    # Export targets
    install(EXPORT betting_by_timeTargets
        FILE betting_by_timeTargets.cmake
        NAMESPACE betting_by_time::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/betting_by_time
    )

    # Export Eigen dependency
    install(TARGETS eigen EXPORT eigenTargets)
    install(EXPORT eigenTargets
        FILE eigenTargets.cmake
        NAMESPACE Eigen3::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/eigen
    )

    # Generate and install package config file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/betting_by_timeConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/betting_by_timeConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/betting_by_timeConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/betting_by_time
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/betting_by_timeConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/betting_by_timeConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/betting_by_time
    )
endif()
