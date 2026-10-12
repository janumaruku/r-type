add_executable(r-type_server_tests main.cpp)

target_link_libraries(r-type_server_tests
    PRIVATE
        GTest::gtest_main
        project_options)

if(WIN32)
    target_compile_options(r-type_server_tests PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-Wno-error=global-constructors>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-error=newline-eof>)
endif()

include(GoogleTest)
gtest_discover_tests(r-type_server_tests)