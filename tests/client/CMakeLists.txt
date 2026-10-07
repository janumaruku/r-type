add_executable(r-type_client_tests main.cpp)

target_link_libraries(r-type_client_tests
    PRIVATE
        GTest::gtest_main
        project_options)

include(GoogleTest)
gtest_discover_tests(r-type_client_tests)