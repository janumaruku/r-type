if (NOT WITH_COVERAGE)
    return()
endif ()

string(REGEX MATCH "^[0-9]+" llvm_major "${CMAKE_CXX_COMPILER_VERSION}")
find_program(LLVM_PROFDATA NAMES llvm-profdata-${llvm_major} llvm-profdata REQUIRED)
find_program(LLVM_COV NAMES llvm-cov-${llvm_major} llvm-cov REQUIRED)

get_property(test_targets GLOBAL PROPERTY RTYPE_TEST_TARGETS)

set(objects "")
foreach (target IN LISTS test_targets)
    list(APPEND objects -object $<TARGET_FILE:${target}>)
endforeach ()

set(profraw_dir "${CMAKE_BINARY_DIR}/profraw")
set(profdata "${CMAKE_BINARY_DIR}/coverage.profdata")
set(html_dir "${CMAKE_BINARY_DIR}/coverage_html")
set(ignored "(\\.cpm-cache|_deps|tests)/")

add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${profraw_dir}
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${profraw_dir}/%m.profraw
        ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} --output-on-failure
        COMMAND ${LLVM_PROFDATA} merge -sparse ${profraw_dir} -o ${profdata}
        COMMAND ${LLVM_COV} report ${objects} -instr-profile=${profdata}
        -ignore-filename-regex=${ignored}
        COMMAND ${LLVM_COV} show ${objects} -instr-profile=${profdata}
        -format=html -output-dir=${html_dir}
        -ignore-filename-regex=${ignored}
        DEPENDS ${test_targets}
        VERBATIM)