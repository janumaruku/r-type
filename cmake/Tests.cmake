include(GoogleTest)

function(rtype_add_test name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LIBS" ${ARGN})
    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE ${ARG_LIBS} GTest::gtest_main project_options)
    gtest_discover_tests(${name})
    set_property(GLOBAL APPEND PROPERTY RTYPE_TEST_TARGETS ${name})
endfunction()
