# Tests CMakeLists.txt

add_executable(betting_tests
    test_types.cpp
    test_utilities.cpp
    test_capital_processes.cpp
    test_betting_strategies.cpp
    test_equivalence.cpp
)

target_link_libraries(betting_tests PRIVATE 
    betting_by_time 
    Catch2::Catch2WithMain
)

# Provide test data directory to tests so they can find generated CSVs
target_compile_definitions(betting_tests PRIVATE
    TEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test_data\"
)

include(Catch)
catch_discover_tests(betting_tests)
