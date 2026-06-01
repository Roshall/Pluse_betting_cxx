# Examples CMakeLists.txt

add_executable(betting_basic_usage
    basic_usage.cpp
)

target_link_libraries(betting_basic_usage PRIVATE 
    betting_by_time
)

add_executable(test_bet_on_estimate
    test_bet_on_estimate.cpp
)

target_link_libraries(test_bet_on_estimate PRIVATE 
    betting_by_time
)

add_executable(test_adaptive_betting
    test_adaptive_betting.cpp
)

target_link_libraries(test_adaptive_betting PRIVATE 
    betting_by_time
)
