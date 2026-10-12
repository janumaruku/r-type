include(CPM)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

CPMAddPackage(
    NAME raylib
    GITHUB_REPOSITORY raysan5/raylib
    GIT_TAG 5.5
    OPTIONS
        "BUILD_EXAMPLES OFF"
        "BUILD_GAMES OFF")

set(GTEST_OPTIONS
    "INSTALL_GTEST OFF"
    "BUILD_GMOCK OFF"
    "gtest_build_tests OFF")

if(WIN32)
    list(APPEND GTEST_OPTIONS "gtest_force_shared_crt ON")
endif()

CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v1.15.2
    OPTIONS ${GTEST_OPTIONS})

function(disable_external_warnings target_name)
    if(TARGET "${target_name}")
        set_property(TARGET "${target_name}" PROPERTY COMPILE_WARNING_AS_ERROR OFF)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options("${target_name}" PRIVATE -w)
        elseif(MSVC)
            target_compile_options("${target_name}" PRIVATE /W0)
        endif()
    endif()
endfunction()

disable_external_warnings(raylib)
disable_external_warnings(glfw)
disable_external_warnings(gtest)
disable_external_warnings(gtest_main)

CPMAddPackage(
    NAME asio
    GITHUB_REPOSITORY chriskohlhoff/asio
    GIT_TAG asio-1-30-2
    DOWNLOAD_ONLY YES)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE "${asio_SOURCE_DIR}/asio/include")
target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)

if(WIN32)
    target_compile_definitions(asio INTERFACE _WIN32_WINNT=0x0A00)
    target_link_libraries(asio INTERFACE ws2_32 mswsock)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(asio INTERFACE Threads::Threads)
endif()
