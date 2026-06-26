# Benchmarks CMakeLists.txt

# Fetch Google Benchmark (only if not already available from parent)
if(NOT TARGET benchmark::benchmark)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing of the benchmark library." FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Enable building gtest-based unit tests." FORCE)
    set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "Download and build missing dependencies." FORCE)

    FetchContent_Declare(
      google_benchmark
      GIT_REPOSITORY https://github.com/google/benchmark.git
      GIT_TAG v1.8.3
    )
    FetchContent_MakeAvailable(google_benchmark)
    target_compile_options(benchmark PUBLIC -Wno-c2y-extensions)
    target_compile_options(benchmark_main PUBLIC -Wno-c2y-extensions)
endif()

add_executable(betting_benchmarks
    benchmark_main.cpp
)

target_link_libraries(betting_benchmarks PRIVATE 
    betting_by_time 
    benchmark::benchmark
)

# Enable performance counters on Linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(betting_benchmarks PRIVATE rt pthread)
endif()
