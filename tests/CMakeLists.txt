add_executable(rpg_tests test_main.cpp)
target_link_libraries(rpg_tests PRIVATE rpg_core doctest::doctest)
add_test(NAME rpg_tests COMMAND rpg_tests)