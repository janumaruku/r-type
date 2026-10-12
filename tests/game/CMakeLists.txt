add_executable(game_tests GameTests.cpp)
target_link_libraries(game_tests PRIVATE rtype_game_domain GTest::gtest_main)
gtest_discover_tests(game_tests)
