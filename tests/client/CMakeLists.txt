add_executable(client_tests ClientTests.cpp)
target_link_libraries(client_tests PRIVATE project_options GTest::gtest_main)
gtest_discover_tests(client_tests)
