# Tests CMakeLists.txt

add_executable(betting_tests
    test_types.cpp
    test_utilities.cpp
    test_capital_processes.cpp
    test_betting_strategies.cpp
)

target_link_libraries(betting_tests PRIVATE 
    betting_by_time 
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(betting_tests)
