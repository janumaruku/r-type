option(WITH_COVERAGE "Enable coverage reporting" OFF)

set(TEST_SOURCES
    client/main.cpp
    server/main.cpp)

add_executable(unit_tests ${TEST_SOURCES})

target_link_libraries(unit_tests
    PRIVATE
        GTest::gtest_main
        project_options)

if(WIN32)
    target_compile_options(unit_tests PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-Wno-error=global-constructors>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-error=newline-eof>)
endif()

include(GoogleTest)
gtest_discover_tests(unit_tests)

if(WITH_COVERAGE)
    find_program(LLVM_PROFDATA NAMES llvm-profdata-20 llvm-profdata)
    find_program(LLVM_COV NAMES llvm-cov-20 llvm-cov)

    if(NOT LLVM_PROFDATA OR NOT LLVM_COV)
        message(WARNING "LLVM coverage tools not found; coverage targets disabled")
    else()
        message(STATUS "LLVM coverage tools found")

        set(COVERAGE_BUILD_DIR "${CMAKE_BINARY_DIR}/coverage")
        set(TEST_PROFILE "${COVERAGE_BUILD_DIR}/unit_tests.profraw")
        set(COVERAGE_DATA_FILE "${COVERAGE_BUILD_DIR}/default.profdata")
        set(COVERAGE_HTML_DIR "${COVERAGE_BUILD_DIR}/html")

        file(MAKE_DIRECTORY "${COVERAGE_BUILD_DIR}")

        target_compile_options(unit_tests PRIVATE
            -fprofile-instr-generate
            -fcoverage-mapping
            -O0
            -g)
        target_link_options(unit_tests PRIVATE
            -fprofile-instr-generate)

        add_custom_target(run_tests_with_coverage
            COMMAND ${CMAKE_COMMAND} -E env
                LLVM_PROFILE_FILE=${TEST_PROFILE}
                $<TARGET_FILE:unit_tests>
            DEPENDS unit_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

        add_custom_target(merge_coverage
            COMMAND ${LLVM_PROFDATA} merge -sparse
                ${TEST_PROFILE}
                -o ${COVERAGE_DATA_FILE}
            DEPENDS run_tests_with_coverage)

        add_custom_target(coverage_report
            COMMAND ${LLVM_COV} report
                $<TARGET_FILE:unit_tests>
                -instr-profile=${COVERAGE_DATA_FILE}
                -ignore-filename-regex=build/_deps
                -ignore-filename-regex=tests/
            DEPENDS merge_coverage)

        add_custom_target(coverage_html
            COMMAND ${LLVM_COV} show
                $<TARGET_FILE:unit_tests>
                -instr-profile=${COVERAGE_DATA_FILE}
                -format=html
                -output-dir=${COVERAGE_HTML_DIR}
                -ignore-filename-regex=build/_deps
                -ignore-filename-regex=tests/
            DEPENDS merge_coverage)

        add_custom_target(coverage
            DEPENDS coverage_report coverage_html)
    endif()
endif()
