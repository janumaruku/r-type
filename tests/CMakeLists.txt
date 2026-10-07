include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

option(WITH_COVERAGE "Enable coverage reporting" OFF)

set(TEST_SOURCES
    client/main.cpp
    server/main.cpp)

add_executable(unit_tests ${TEST_SOURCES})

target_link_libraries(unit_tests
    PRIVATE
        GTest::gtest_main
        project_options)

set_target_properties(unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

#target_compile_options(unit_tests PRIVATE --coverage -O0 -g)
#target_link_options(unit_tests PRIVATE --coverage)

if (WITH_COVERAGE)
    message(STATUS "Coverage enabled")

    target_compile_options(unit_tests PRIVATE
            -fprofile-instr-generate
            -fcoverage-mapping
            -O0 -g
    )
    target_link_options(unit_tests PRIVATE
            -fprofile-instr-generate
    )
endif()

# Detect matching LLVM coverage tools
execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} --version
        OUTPUT_VARIABLE CLANG_VERSION_OUTPUT
)

string(REGEX MATCH "clang version ([0-9]+)" _ ${CLANG_VERSION_OUTPUT})
set(CLANG_MAJOR_VERSION ${CMAKE_MATCH_1})

set(LLVM_PROFDATA llvm-profdata-20)
set(LLVM_COV llvm-cov-20)

set(HAVE_LLVM_COVERAGE OFF)
if(NOT LLVM_PROFDATA OR NOT LLVM_COV)
    message(WARNING "LLVM 18 coverage tools not found")
else ()
    set(HAVE_LLVM_COVERAGE ON)
    message(STATUS "LLVM coverage tools found: ${LLVM_PROFDATA}, ${LLVM_COV}")
endif()

if (HAVE_LLVM_COVERAGE)

    # Where profile data will be written
    set(COVERAGE_RAW_FILE ${CMAKE_BINARY_DIR}/default.profraw)
    set(COVERAGE_DATA_FILE ${CMAKE_BINARY_DIR}/default.profdata)
    set(COVERAGE_HTML_DIR ${CMAKE_BINARY_DIR}/coverage_html)

    # Ensure tests write profile data to predictable location
    add_custom_target(run_tests_with_coverage
            COMMAND ${CMAKE_COMMAND} -E env
            LLVM_PROFILE_FILE=${COVERAGE_RAW_FILE}
            $<TARGET_FILE:unit_tests>
            DEPENDS unit_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Merge raw profile into profdata
    add_custom_target(merge_coverage
            COMMAND ${LLVM_PROFDATA} merge -sparse
            ${COVERAGE_RAW_FILE}
            -o ${COVERAGE_DATA_FILE}
            DEPENDS run_tests_with_coverage
    )

    # Generate terminal report
    add_custom_target(coverage_report
            COMMAND ${LLVM_COV} report
            $<TARGET_FILE:unit_tests>
            -instr-profile=${COVERAGE_DATA_FILE}
            -compilation-dir=${CMAKE_SOURCE_DIR}
            -ignore-filename-regex='build/_deps'
            -ignore-filename-regex='tests/'
            DEPENDS merge_coverage
    )

    # Generate HTML report
    add_custom_target(coverage_html
            COMMAND ${LLVM_COV} show
            $<TARGET_FILE:unit_tests>
            -instr-profile=${COVERAGE_DATA_FILE}
            -format=html
            -output-dir=${COVERAGE_HTML_DIR}
            -compilation-dir=${CMAKE_SOURCE_DIR}
            -path-equivalence=${CMAKE_SOURCE_DIR},.
            -ignore-filename-regex='build/_deps'
            -ignore-filename-regex='tests/'
            DEPENDS merge_coverage
    )

    # Master target
    add_custom_target(coverage
            DEPENDS coverage_report coverage_html
    )

endif()

include(GoogleTest)
gtest_discover_tests(unit_tests)